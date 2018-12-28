#include "connection.h"
#include "../lib/console_app_tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

	new_connection->request_url = NULL;
	new_connection->request_url_is_set = 0;
	new_connection->is_bad_protocol = 0;
	new_connection->is_get_request = 0;
	new_connection->response_is_ok = 0;
	new_connection->response_data = NULL;
	new_connection->response_data_length = 0;

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

char buf[1024];

void connection_parse_request(Connection *connection)
{
	size_t buf_length = connection->client_to_backend_bytes_count < sizeof(buf)
							? connection->client_to_backend_bytes_count
							: sizeof(buf);
	memcpy(buf, connection->client_to_backend_bytes, buf_length);
	// METHOD url protocol -> url
	char *after_method = memchr(buf, ' ', buf_length);
	if (!after_method)
	{
		connection->is_bad_protocol = 1;
		return;
	}
	*after_method = '\0';
	if (strcmp(buf, "GET"))
	{
		connection->request_url_is_set = 1;
		return;
	}
	char *after_url = memchr(after_method + 1, ' ',
							 buf_length - (size_t)(after_url - buf));
	if (!after_url)
	{
		connection->is_bad_protocol = 1;
		return;
	}
	*after_url = '\0';
	size_t request_size = (size_t)(after_url - after_method + 1);
	connection->request_url = malloc(request_size);
	memcpy(connection->request_url, after_method + 1, request_size);
	printf("%sConnection #%d requested '%s'\n", BLUE_COLOR,
		   connection->id, connection->request_url);
	connection->is_get_request = 1;
	connection->request_url_is_set = 1;
}

void connection_parse_response(Connection *connection)
{
	size_t buf_length = connection->response_data_length < sizeof(buf)
							? connection->response_data_length
							: sizeof(buf);
	memcpy(buf, connection->response_data, buf_length);
	// protocol status CODE -> status
	char *after_protocol = memchr(buf, ' ', buf_length);
	if (!after_protocol)
		return;
	char *after_status = memchr(after_protocol + 1, ' ',
								buf_length - (size_t)(after_protocol - buf));
	if (!after_status)
		return;
	*after_status = '\0';
	if (!strcmp(after_protocol + 1, "200"))
		connection->response_is_ok = 1;
}

void connection_drop(Connection *connection, Connection **connections)
{
	if (connection == (*connections))
		(*connections) = connection->next;
	else
		connection->prev->next = connection->next;
	if (connection->next)
		connection->next->prev = connection->prev;

	shutdown(connection->client_fd, SHUT_RDWR);
	close(connection->client_fd);
	shutdown(connection->backend_fd, SHUT_RDWR);
	close(connection->backend_fd);
	if (connection->request_url)
		free(connection->request_url);
	if (connection->response_data)
		free(connection->response_data);
	free(connection);
}