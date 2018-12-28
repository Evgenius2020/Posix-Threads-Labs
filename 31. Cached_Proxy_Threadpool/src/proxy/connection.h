#ifndef CONNECTION
#define CONNECTION

#include <time.h>

#define BUFSIZE 1024

typedef struct Connection
{
	unsigned id;
	struct Connection *prev, *next;

	char *request_url;
	int request_url_is_set;
	int is_bad_protocol;
	int is_get_request;
	int response_is_ok;
	char *response_data;
	size_t response_data_length;

	int client_fd;
	int backend_fd;

	char client_to_backend_bytes[BUFSIZE];
	char backend_to_client_bytes[BUFSIZE];
	size_t client_to_backend_bytes_count;
	size_t backend_to_client_bytes_count;

	int is_broken;
	int is_loaded_from_cache;

	time_t last_update;
} Connection;

Connection *connection_create(int client_fd, int backend_fd, Connection **connections);
void connection_parse_request(Connection *connection);
void connection_parse_response(Connection *connection);
void connection_drop(Connection *connection, Connection **connections);

#endif