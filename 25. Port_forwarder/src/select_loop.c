#include "connection.h"
#include "console_app_tools.h"

#define NO_UPDATE_LIMIT_SEC 600

void time_or_expect(time_t *tloc)
{
    if (-1 == time(tloc))
        throw_and_exit("time");
}

void select_loop(Connection **connections,
                 int frontend_fd, void (*on_client_connect)(), int max_fd)
{
    fd_set readfds, writefds;
    int ready, read;
    char buf[BUFSIZE];
    Connection *connection;
    time_t time_now;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(frontend_fd, &readfds);

        for (Connection *connection = *connections; connection; connection = connection->next)
        {
            time_or_expect(&time_now);

            if ((connection->cnt_cs < 0 && connection->cnt_sc <= 0) ||
                (connection->cnt_sc < 0 && connection->cnt_cs <= 0))
                connection_drop(connection, connections);
            else if (time_now - connection->last_update > NO_UPDATE_LIMIT_SEC)
                connection_drop(connection, connections);
            else
            {
                if (connection->cnt_cs == 0)
                    FD_SET(connection->client_fd, &readfds);
                if (connection->cnt_sc == 0)
                    FD_SET(connection->backend_fd, &readfds);
                if (connection->cnt_cs > 0)
                    FD_SET(connection->backend_fd, &writefds);
                if (connection->cnt_sc > 0)
                    FD_SET(connection->client_fd, &writefds);
            }
        }

        ready = select(max_fd + 1, &readfds, &writefds, NULL, NULL);
        if (-1 == ready)
            throw_and_exit("select");
        else if (!ready)
            continue;

        for (Connection *connection = *connections; connection; connection = connection->next)
        {
            // Client -> Frontend
            if (connection->cnt_cs == 0 && FD_ISSET(connection->client_fd, &readfds))
            {
                connection->cnt_cs = recv(connection->client_fd, connection->data_cs, sizeof(connection->data_cs), 0);
                if (connection->cnt_cs == 0)
                    connection->cnt_cs = -1;
            }

            // Frontend -> Backend
            if (connection->cnt_sc == 0 && FD_ISSET(connection->backend_fd, &readfds))
            {
                connection->cnt_sc = recv(connection->backend_fd, connection->data_sc, sizeof(connection->data_sc), 0);
                if (connection->cnt_sc == 0)
                    connection->cnt_sc = -1;
            }

            // Backend -> Frontend
            if (connection->cnt_cs > 0 && FD_ISSET(connection->backend_fd, &writefds))
            {
                int res = send(connection->backend_fd, connection->data_cs, connection->cnt_cs, 0);
                if (res == -1)
                    connection->cnt_sc = -1;
                else
                    connection->cnt_cs = 0;
            }

            // Backend -> Client
            if (connection->cnt_sc > 0 && FD_ISSET(connection->client_fd, &writefds))
            {
                int res = send(connection->client_fd, connection->data_sc, connection->cnt_sc, 0);
                if (res == -1)
                    connection->cnt_cs = -1;
                else
                    connection->cnt_sc = 0;
            }
            time_or_expect(&connection->last_update);
        }

        // Client connect
        if (FD_ISSET(frontend_fd, &readfds))
            on_client_connect();
    }
}