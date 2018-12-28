#include "hashmap.h"
#include "lib/cond.h"
#include "lib/console_app_tools.h"
#include "lib/mutex.h"
#include "proxy/connection.h"
#include "thread_definition.h"
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int frontend_fd;
int is_disposed = 0;
struct addrinfo *backend_ai;
pthread_mutex_t cache_mutex;
Hashmap cache;
int threadpool_size;
pthread_t *threadpool;
Thread_Definition *threadpool_defs;

// select_loop.c
// Updating select mask and select processing.
void *select_loop(void *thread_definition_raw);

void getaddrinfo_or_except(char *url, char *port, struct addrinfo *hints, struct addrinfo **res)
{
	int ret = getaddrinfo(url, port, hints, res);
	if (ret)
	{
		fprintf(stderr, "%sFailed to getaddrinfo: %s\n", RED_COLOR, gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
}

int get_socket_fd_or_except(struct addrinfo *ai,
							int (*bind_or_connect)(int, const struct sockaddr *, socklen_t))
{
	while (1)
	{
		if (!ai)
			throw_and_exit("get_socket_fd");
		int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (-1 == fd)
			continue;

		if (!bind_or_connect(fd, ai->ai_addr, ai->ai_addrlen))
			return fd;
		close(fd);
		ai = ai->ai_next;
	}
}

void on_client_connect()
{
	int client_fd = accept(frontend_fd, NULL, NULL);
	if (-1 == client_fd)
		throw_and_exit("accept");
	int backend_fd = get_socket_fd_or_except(backend_ai, connect);
}

void at_exit()
{
	if (is_disposed)
		return;
	is_disposed = 1;

	for (int i = 0; i < threadpool_size; i++)
	{
		pthread_cancel(threadpool[i]);
		while (threadpool_defs->connections)
		{
			Connection *next = threadpool_defs->connections->next;
			free(threadpool_defs->connections);
			threadpool_defs->connections = next;
		}
	}
	free(threadpool_defs);
	free(threadpool);
	hashmap_dispose(&cache);

	freeaddrinfo(backend_ai);
	shutdown(frontend_fd, SHUT_RDWR);
	close(frontend_fd);

	printf("%sStopped listenning.\n", GREEN_COLOR);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		fprintf(stderr, "Usage: %s [threadpool_size] [frontend_port] [backend_url] [backend_port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	threadpool_size = atoi(argv[1]);
	if (!threadpool_size)
	{
		fprintf(stderr, "Bad threadpool_size\n");
		exit(EXIT_FAILURE);
	}
	char *frontend_port = argv[2];
	char *backend_url = argv[3];
	char *backend_port = argv[4];

	atexit(at_exit);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, at_exit);
	signal(SIGTERM, at_exit);

	hashmap_init(&cache);
	mutex_try_init(&cache_mutex);
	threadpool = malloc(sizeof(pthread_t) * threadpool_size);
	threadpool_defs = malloc(sizeof(Thread_Definition) * threadpool_size);
	for (int i = 0; i < threadpool_size; i++)
	{
		threadpool_defs[i].cache = &cache;
		threadpool_defs[i].cache_mutex = &cache_mutex;
		threadpool_defs[i].connections = NULL;
		threadpool_defs[i].connections_length = 0;
		cond_try_init(&threadpool_defs[i].connections_cond);
		mutex_try_init(&threadpool_defs[i].connections_mutex);
		if (-1 == pthread_create(&threadpool[i], NULL, select_loop, &threadpool_defs[i]))
			throw_and_exit("pthread_create");
	}
	printf("%sThreadpool created\n", YELLOW_COLOR);

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
	printf("%sStarted listening\n", YELLOW_COLOR);

	while (1)
	{
		int client_fd = accept(frontend_fd, NULL, NULL);
		if (-1 == client_fd)
			throw_and_exit("accept");
		int backend_fd = get_socket_fd_or_except(backend_ai, connect);

		int min_connections_thread_id = -1;
		int min_connections = -1;
		for (int i = 0; i < threadpool_size; i++)
		{
			mutex_try_lock(&threadpool_defs[i].connections_mutex);
			if (min_connections_thread_id == -1 ||
				threadpool_defs[i].connections_length < min_connections)
			{
				min_connections = threadpool_defs[i].connections_length;
				min_connections_thread_id = i;
			}
			mutex_try_unlock(&threadpool_defs[i].connections_mutex);
		}
		Thread_Definition *thread_def = &threadpool_defs[min_connections_thread_id];
		mutex_try_lock(&thread_def->connections_mutex);
		connection_create(client_fd, backend_fd, &thread_def->connections);
		thread_def->connections_length++;
		cond_try_signal(&thread_def->connections_cond);
		mutex_try_unlock(&thread_def->connections_mutex);
	}

	exit(EXIT_SUCCESS);
}