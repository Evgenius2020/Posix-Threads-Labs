#include "connection.h"
#include "../lib/console_app_tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>


// Debug
void print_list(Connection *connection)
{
	printf("HEAD -> ");
	for (; connection; connection = connection->next)
		printf("%d -> ", connection->id);
	printf("NULL\n");
}

Connection *connection_create(int client_fd, int backend_fd, Connection **connections)
{
	Connection *new_connection;
	new_connection = malloc(sizeof(Connection));
	new_connection->request_is_set = 0;
	new_connection->client_fd = client_fd;
	new_connection->backend_fd = backend_fd;
	new_connection->client_to_backend_bytes_count = 0;
	new_connection->backend_to_client_bytes_count = 0;
	new_connection->last_update = time(&(new_connection->last_update));
	new_connection->id = backend_fd;
	new_connection->is_broken = 0;

	new_connection->prev = NULL;
	new_connection->next = *connections;
	if ((*connections))
		(*connections)->prev = new_connection;
	*connections = new_connection;

	printf("%sCreated connection #%d\n", YELLOW_COLOR, new_connection->id);

	return new_connection;
}

void connection_drop(Connection *connection, Connection **connections)
{
	if (connection == (*connections))
		(*connections) = connection->next;
	else
		connection->prev->next = connection->next;
	if (connection->next)
		connection->next->prev = connection->prev;

	printf("%sDropped connection #%d\n", GREEN_COLOR, connection->id);

	shutdown(connection->client_fd, SHUT_RDWR);
	close(connection->client_fd);
	shutdown(connection->backend_fd, SHUT_RDWR);
	close(connection->backend_fd);
	free(connection);
}