#include "connection.h"
#include "console_app_tools.h"
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define NO_UPDATE_LIMIT_SEC 600

int frontend_fd;
int fd_max = 0;
struct addrinfo *backend_ai;
Connection_List connections;

int update_fd_max(int fd)
{
	if (fd >= FD_SETSIZE)
		return -1;
	if (fd > fd_max)
		fd_max = fd;
	return 0;
}

void getaddrinfo_or_except(char *url, char *port, struct addrinfo *hints, struct addrinfo **res)
{
	int ret = getaddrinfo(url, port, hints, res);
	if (ret)
	{
		fprintf(stderr, "%sFailed to getaddrinfo: %s\n", RED_COLOR, gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
}

void set_select_mask(fd_set *readfds, fd_set *writefds)
{
	Connection *connection;
	connection = connections.first;
	FD_ZERO(readfds);
	FD_ZERO(writefds);
	FD_SET(frontend_fd, readfds);

	while (connection)
	{
		time_t time_now = time(&time_now);
		if (-1 == time_now)
			throw_and_exit("time");

		if (connection->cnt_cs <= 0 && connection->cnt_sc <= 0)
			connection_drop(connection, &connections);
		else if (time_now - connection->last_update > NO_UPDATE_LIMIT_SEC)
			connection_drop(connection, &connections);
		else
		{
			if (connection->cnt_cs == 0)
				FD_SET(connection->client_fd, readfds);
			if (connection->cnt_sc == 0)
				FD_SET(connection->backend_fd, readfds);
			if (connection->cnt_cs > 0)
				FD_SET(connection->backend_fd, writefds);
			if (connection->cnt_sc > 0)
				FD_SET(connection->client_fd, writefds);
		}
		connection = connection->next;
	}
}

int get_socket_fd_or_except(struct addrinfo *ai, 
	int (*bind_or_connect)(int, const struct sockaddr*, socklen_t))
{
	int fd;
	struct addrinfo *ai_buf;
	for (ai_buf = ai; ai_buf; ai_buf = ai_buf->ai_next)
	{
		fd = socket(ai_buf->ai_family, ai_buf->ai_socktype, ai_buf->ai_protocol);
		if (-1 == fd)
			continue;

		if (!bind_or_connect(fd, ai_buf->ai_addr, ai_buf->ai_addrlen))
			break;
		close(fd);
	}
	if (!ai_buf)
		throw_and_exit("get_socket_fd");

	return fd;
}

void at_close()
{
	freeaddrinfo(backend_ai);
	close(frontend_fd);
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s [frontend_port] [backend_url] [backend_port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	atexit(at_close);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, at_close);
	signal(SIGTERM, at_close);

	char *frontend_port = argv[1];
	char *backend_url = argv[2];
	char *backend_port = argv[3];

	struct addrinfo hints, *frontend_ai;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo_or_except(backend_url, backend_port, &hints, &backend_ai);

	hints.ai_flags = AI_PASSIVE;
	getaddrinfo_or_except(NULL, frontend_port, &hints, &frontend_ai);
	frontend_fd = get_socket_fd_or_except(frontend_ai, bind);
	freeaddrinfo(frontend_ai);
	if (listen(frontend_fd, SOMAXCONN))
		throw_and_exit("listen");
	if (update_fd_max(frontend_fd))
		throw_and_exit("update_fd_max");

	fd_set readfds, writefds;
	int ready, read;
	char buf[BUFSIZE];

	while (1)
	{
		set_select_mask(&readfds, &writefds);
		ready = select(fd_max + 1, &readfds, &writefds, NULL, NULL);

		if (-1 == ready)
			throw_and_exit("select");
		else if (!ready)
			continue;

		for (Connection *connection = connections.first; connection; connection = connection->next)
		{
			// Client -> Frontend
			if (connection->cnt_cs == 0 && FD_ISSET(connection->client_fd, &readfds))
			{
				connection->cnt_cs = recv(connection->client_fd, connection->data_cs, sizeof(connection->data_cs), 0);
				connection->last_update = time(&(connection->last_update));
				if (connection->cnt_cs == 0)
					connection->cnt_cs = -1;
			}

			// Frontend -> Backend
			if (connection->cnt_sc == 0 && FD_ISSET(connection->backend_fd, &readfds))
			{
				connection->cnt_sc = recv(connection->backend_fd, connection->data_sc, sizeof(connection->data_sc), 0);
				connection->last_update = time(&(connection->last_update));
				if (connection->cnt_sc == 0)
					connection->cnt_sc = -1;
			}

			// Backend -> Frontend
			if (connection->cnt_cs > 0 && FD_ISSET(connection->backend_fd, &writefds))
			{
				int res = send(connection->backend_fd, connection->data_cs, connection->cnt_cs, 0);
				connection->last_update = time(&(connection->last_update));
				if (res == -1)
					connection->cnt_sc = -1;
				else
					connection->cnt_cs = 0;
			}

			// Backend -> Client
			if (connection->cnt_sc > 0 && FD_ISSET(connection->client_fd, &writefds))
			{
				int res = send(connection->client_fd, connection->data_sc, connection->cnt_sc, 0);
				connection->last_update = time(&(connection->last_update));
				if (res == -1)
					connection->cnt_cs = -1;
				else
					connection->cnt_sc = 0;
			}
		}

		// Client connect
		if (FD_ISSET(frontend_fd, &readfds))
		{
			int client_fd = accept(frontend_fd, NULL, NULL);
			if (-1 == client_fd)
				throw_and_exit("accept");
			if (update_fd_max(client_fd))
				throw_and_exit("update_fd_max");
			int backend_fd = get_socket_fd_or_except(backend_ai, connect);
			if (!connection_create(client_fd, backend_fd, &connections))
				throw_and_exit("connection_create");
		}
	}

	exit(EXIT_SUCCESS);
}