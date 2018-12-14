#ifndef LIST
#define LIST

typedef struct node
{
	char *value;
	struct node *next;
} node;

node *list_add_node(node *last, char *value);
void list_destroy(node *first);
void list_print(node *first);

#endif