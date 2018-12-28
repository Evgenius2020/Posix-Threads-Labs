#ifndef CONNECTION
#define CONNECTION

#include <stdlib.h>

typedef struct Connection
{
	unsigned id;

	char *request_url;
	int request_url_is_set;
	int is_bad_protocol;
	int is_get_request;
	int response_is_ok;
	char *response_data;
	size_t response_data_length;

	int client_fd;
	int backend_fd;

	int is_broken;
} Connection;

Connection *connection_create(int client_fd, int backend_fd);
void connection_parse_request(Connection *connection, char *data, size_t data_len);
void connection_parse_response(Connection *connection, char *data, size_t data_len);
void connection_drop(Connection *connection);

#endif