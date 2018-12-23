#include "connection.h"
#include "console_app_tools.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

Connection *connection_create(int client_fd, int backend_fd, Connection_List *connections)
{
	Connection *new_connection;
	new_connection = malloc(sizeof(Connection));
	new_connection->client_fd = client_fd;
	new_connection->backend_fd = backend_fd;
	new_connection->cnt_sc = 0;
	new_connection->cnt_cs = 0;
	new_connection->last_update = time(&(new_connection->last_update));

	new_connection->prev = NULL;
	new_connection->next = connections->first;
	if (connections->first == NULL)
		connections->last = new_connection;
	else
		connections->first->prev = new_connection;
	connections->first = new_connection;

	printf("%sCreated connection #%d\n", YELLOW_COLOR, new_connection->backend_fd);

	return new_connection;
}

void connection_drop(Connection *connection, Connection_List *connections)
{
	if (connection == connections->first && connection == connections->last)
	{
		connections->first = connections->last = NULL;
	}
	else if (connection == connections->first)
	{
		connections->first = connection->next;
		connections->first->prev = NULL;
	}
	else if (connection == connections->last)
	{
		connections->last = connection->prev;
		connections->last->next = NULL;
	}
	else
	{
		connection->prev->next = connection->next;
		connection->next->prev = connection->prev;
	}
	printf("Connection dropped\n");
	close(connection->client_fd);
	close(connection->backend_fd);
	free(connection);
}