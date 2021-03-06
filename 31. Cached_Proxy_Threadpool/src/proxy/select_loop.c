#include "../hashmap.h"
#include "../lib/cond.h"
#include "../lib/console_app_tools.h"
#include "../lib/mutex.h"
#include "../thread_definition.h"
#include "connection.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define NO_UPDATE_LIMIT_SEC 5

void time_or_expect(time_t *tloc)
{
    if (-1 == time(tloc))
        throw_and_exit("time");
}

void update_max_fd_or_except(int fd, int *max_fd)
{
    if (fd + 1 >= FD_SETSIZE)
    {
        fprintf(stderr, "%sFailed to update_max_fd: fd > FD_SETSIZE\n", RED_COLOR);
        exit(EXIT_FAILURE);
    }
    if (fd > *max_fd)
        *max_fd = fd;
}

void *select_loop(void *thread_definition_raw)
{
    Thread_Definition *thread_def = (Thread_Definition *)thread_definition_raw;
    fd_set readfds, writefds;
    time_t time_now;

    while (1)
    {
        mutex_try_lock(&thread_def->connections_mutex);
        while (!thread_def->connections)
            cond_try_wait(&thread_def->connections_cond, &thread_def->connections_mutex);
        Connection *connection = thread_def->connections;
        mutex_try_unlock(&thread_def->connections_mutex);

        int max_fd = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        while (connection) // Don't use 'for' (drop modifies structure of list)
        {
            time_or_expect(&time_now);

            if (connection->is_broken ||
                time_now - connection->last_update > NO_UPDATE_LIMIT_SEC)
            {
                if (connection->is_broken)
                    printf("%sDropped connection #%d\n", GREEN_COLOR, connection->id);
                else
                {
                    if (connection->is_get_request)
                    {
                        mutex_try_lock(thread_def->cache_mutex);
                        connection_parse_response(connection);
                        if (connection->response_is_ok &&
                            !hashmap_get(thread_def->cache, connection->request_url))
                        {
                            hashmap_insert(
                                thread_def->cache,
                                connection->request_url,
                                connection->response_data,
                                connection->response_data_length);
                            printf("%s'%s' is cached\n", MAGENTA_COLOR, connection->request_url);
                        }
                        mutex_try_unlock(thread_def->cache_mutex);
                    }
                    printf("%sClosed connection #%d\n", GREEN_COLOR, connection->id);
                }

                Connection *buf = connection->next;
                mutex_try_lock(&thread_def->connections_mutex);
                connection_drop(connection, &thread_def->connections);
                thread_def->connections_length--;
                mutex_try_unlock(&thread_def->connections_mutex);
                connection = buf;
            }
            else
            {
                update_max_fd_or_except(connection->client_fd, &max_fd);
                mutex_try_lock(thread_def->cache_mutex);
                if (connection->request_url_is_set &&
                    connection->is_get_request &&
                    hashmap_get(thread_def->cache, connection->request_url) &&
                    !connection->is_loaded_from_cache)
                    FD_SET(connection->client_fd, &writefds);
                else
                {
                    update_max_fd_or_except(connection->backend_fd, &max_fd);
                    if (connection->client_to_backend_bytes_count)
                        FD_SET(connection->backend_fd, &writefds);
                    else
                        FD_SET(connection->client_fd, &readfds);

                    if (connection->backend_to_client_bytes_count)
                        FD_SET(connection->client_fd, &writefds);
                    else
                        FD_SET(connection->backend_fd, &readfds);
                }
                mutex_try_unlock(thread_def->cache_mutex);
                connection = connection->next;
            }
        }
        if (!thread_def->connections)
            continue;

        int ready = select(max_fd + 1, &readfds, &writefds, NULL, NULL);
        if (-1 == ready)
            throw_and_exit("select");
        else if (!ready)
            continue;

        mutex_try_lock(&thread_def->connections_mutex);
        connection = thread_def->connections;
        mutex_try_unlock(&thread_def->connections_mutex);
        for (; connection; connection = connection->next)
        {
            int connection_is_updated = 0;
            // Client -> Frontend
            if (FD_ISSET(connection->client_fd, &readfds))
            {
                int bytes_received =
                    recv(connection->client_fd, connection->client_to_backend_bytes,
                         sizeof(connection->client_to_backend_bytes), 0);
                connection->client_to_backend_bytes_count = bytes_received;

                if (!connection->request_url_is_set && !connection->is_bad_protocol)
                {
                    connection_parse_request(connection);
                    if (connection->is_get_request)
                        continue;
                }

                if (bytes_received)
                    connection_is_updated = 1;
            }

            // Frontend -> Backend
            if (FD_ISSET(connection->backend_fd, &writefds))
            {
                if (-1 == send(connection->backend_fd, connection->client_to_backend_bytes,
                               connection->client_to_backend_bytes_count, 0))
                {
                    connection->is_broken = 1;
                    continue;
                }
                connection->client_to_backend_bytes_count = 0;
                connection_is_updated = 1;
            }

            // Backend -> Frontend
            if (FD_ISSET(connection->backend_fd, &readfds))
            {
                int bytes_received =
                    recv(connection->backend_fd, connection->backend_to_client_bytes,
                         sizeof(connection->backend_to_client_bytes), 0);
                connection->backend_to_client_bytes_count = bytes_received;

                if (connection->is_get_request)
                {
                    connection->response_data = realloc(
                        connection->response_data,
                        connection->response_data_length +
                            connection->backend_to_client_bytes_count);
                    memcpy(
                        connection->response_data + connection->response_data_length,
                        connection->backend_to_client_bytes,
                        connection->backend_to_client_bytes_count);
                    connection->response_data_length +=
                        connection->backend_to_client_bytes_count;
                }

                if (bytes_received)
                    connection_is_updated = 1;
            }

            // Backend -> Client
            if (FD_ISSET(connection->client_fd, &writefds))
            {

                if (connection->request_url_is_set &&
                    connection->is_get_request &&
                    !connection->is_loaded_from_cache)
                {
                    mutex_try_lock(thread_def->cache_mutex);
                    Hashmap_List_Element *cache_page =
                        hashmap_get(thread_def->cache, connection->request_url);
                    mutex_try_unlock(thread_def->cache_mutex);
                    if (cache_page)
                    {
                        connection->is_loaded_from_cache = 1;
                        printf("%s'%s' send from cache\n", MAGENTA_COLOR, connection->request_url);
                        send(connection->client_fd, cache_page->value,
                             cache_page->value_size, 0);
                        continue;
                    }
                }

                if (-1 == send(connection->client_fd, connection->backend_to_client_bytes,
                               connection->backend_to_client_bytes_count, 0))
                {
                    connection->is_broken = 1;
                    continue;
                }

                connection->backend_to_client_bytes_count = 0;
                connection_is_updated = 1;
            }

            if (connection_is_updated)
                time_or_expect(&connection->last_update);
        }
    }
}