#ifndef LIST
#define LIST

typedef struct Node
{
	char *value;
	struct Node *next;
} Node;

Node *list_add_node(Node *last, char *value);
void list_destroy(Node *first);
void list_print(Node *first);

#endif