#include "../hashmap.h"
#include "../lib/console_app_tools.h"
#include "../lib/mutex.h"
#include "connection.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Hashmap *cache;
pthread_mutex_t mutex;

void on_thread_stop(void *connection)
{
    int is_broken = ((Connection *)connection)->is_broken;
    int id = ((Connection *)connection)->id;
    connection_drop(connection);
    if (is_broken)
        printf("%sDropped connection #%d\n", GREEN_COLOR, id);
    else
        printf("%sClosed connection #%d\n", GREEN_COLOR, id);
}

void *connection_routine(void *connection_raw)
{
    char buf[2048];
    Connection *connection = (Connection *)connection_raw;

    pthread_cleanup_push(on_thread_stop, connection);
    // Client -> Frontend -> Backend
    while (1)
    {
        int bytes_received = read(connection->client_fd, buf, sizeof(buf));
        if (-1 == bytes_received)
        {
            connection->is_broken = 1;
            pthread_exit(NULL);
        }
        if (!bytes_received)
            break;

        if (!connection->request_url_is_set && !connection->is_bad_protocol)
            connection_parse_request(connection, buf, bytes_received);

        if (-1 == send(connection->backend_fd, buf, bytes_received, 0))
        {
            connection->is_broken = 1;
            pthread_exit(NULL);
        }

        if (bytes_received < sizeof(buf))
            break;
    }

    if (connection->is_get_request)
    {
        mutex_try_lock(&mutex);
        Hashmap_List_Element *cache_page = hashmap_get(cache, connection->request_url);
        mutex_try_unlock(&mutex);
        if (cache_page)
        {
            printf("%s'%s' send from cache\n", MAGENTA_COLOR, connection->request_url);
            send(connection->client_fd, cache_page->value, cache_page->value_size, 0);
            pthread_exit(NULL);
        }
    }

    // Backend -> Frontend -> Client
    while (1)
    {
        int bytes_received = recv(connection->backend_fd, buf, sizeof(buf), 0);
        if (-1 == bytes_received)
        {
            connection->is_broken = 1;
            pthread_exit(NULL);
        }
        if (!bytes_received)
            break;

        if (connection->is_get_request)
        {
            connection->response_data = realloc(
                connection->response_data,
                connection->response_data_length + bytes_received);
            memcpy(
                connection->response_data + connection->response_data_length,
                buf, bytes_received);
            connection->response_data_length += bytes_received;
        }

        if (-1 == send(connection->client_fd, buf, bytes_received, 0))
        {
            connection->is_broken = 1;
            pthread_exit(NULL);
        }
    }

    if (connection->is_get_request)
    {
        connection_parse_response(connection,
                                  connection->response_data, connection->response_data_length);
        mutex_try_lock(&mutex);
        if (connection->response_is_ok && !hashmap_get(cache, connection->request_url))
        {
            hashmap_insert(
                cache,
                connection->request_url,
                connection->response_data,
                connection->response_data_length);
            printf("%s'%s' is cached\n", MAGENTA_COLOR, connection->request_url);
        }
        mutex_try_unlock(&mutex);
    }

    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}

void listen_loop(int frontend_fd, void (*get_new_connection_fds)(int *, int *), Hashmap *cache_ptr)
{
    int client_fd, backend_fd;
    cache = cache_ptr;
    mutex_try_init(&mutex);

    while (1)
    {
        get_new_connection_fds(&client_fd, &backend_fd);
        Connection *connection = connection_create(client_fd, backend_fd);

        pthread_t thread;
        if (pthread_create(&thread, NULL, connection_routine, connection))
            throw_and_exit("pthread_create");
    }
}