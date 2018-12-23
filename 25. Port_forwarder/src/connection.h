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

typedef struct
{
	Connection *first;
	Connection *last;
} Connection_List;

Connection *connection_create(int client_fd, int backend_fd, Connection_List *connections);
void connection_drop(Connection *connection, Connection_List* connections);