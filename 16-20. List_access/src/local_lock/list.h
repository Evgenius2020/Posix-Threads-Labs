#ifndef LIST
#define LIST

#include <pthread.h>

typedef struct Node
{
	char *value;
	struct Node *next;
	pthread_rwlock_t rwlock;
} Node;

Node *list_add_node(Node *last, char *value);
int list_cmp(Node *left, Node *right);
void list_swap(Node *prev, Node *curr, Node *next);
void list_print(Node *first);
void list_destroy(Node *first);

#endif