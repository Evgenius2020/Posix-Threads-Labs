#ifndef CONNECTION
#define CONNECTION

#include <netinet/in.h>
#include <time.h>

#define BUFSIZE 4096

typedef struct Connection
{
	int client_fd;
	int backend_fd;
	unsigned id;
	struct Connection *prev, *next;
	size_t cnt_cs, cnt_sc;
	char data_cs[BUFSIZE];
	char data_sc[BUFSIZE];
	time_t last_update;
} Connection;

Connection *connection_create(int client_fd, int backend_fd, Connection **connections);
void connection_drop(Connection *connection, Connection **connections);

#endif