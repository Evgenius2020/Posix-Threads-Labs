#include "connection.h"
#include "console_app_tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

Connection *connection_create(int client_fd, int backend_fd, Connection **connections)
{
	Connection *new_connection;
	new_connection = malloc(sizeof(Connection));
	new_connection->client_fd = client_fd;
	new_connection->backend_fd = backend_fd;
	new_connection->cnt_sc = 0;
	new_connection->cnt_cs = 0;
	new_connection->last_update = time(&(new_connection->last_update));

	new_connection->prev = NULL;
	new_connection->next = (*connections);
	if ((*connections))
		(*connections)->prev = new_connection;
	*connections = new_connection;

	printf("%sCreated connection #%d\n", YELLOW_COLOR, new_connection->backend_fd);

	return new_connection;
}

void connection_drop(Connection *connection, Connection **connections)
{
	if (connection == (*connections))
		(*connections) = NULL;
	else
		connection->prev->next = connection->next;
	if (connection->next)
		connection->next->prev = connection->prev;

	printf("%sDropped connection #%d\n", YELLOW_COLOR, connection->id);
	close(connection->client_fd);
	close(connection->backend_fd);
	free(connection);
}