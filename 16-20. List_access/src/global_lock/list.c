#include "list.h"
#include <stdio.h>
#include <stdlib.h>

Node *list_add_node(Node *head, char *value)
{
	Node *new_node = (Node *)malloc(sizeof(Node));
	new_node->value = value;
	new_node->next = head;
	return new_node;
}

void list_destroy(Node *head)
{
	Node *buf;
	while (head)
	{
		buf = head;
		head = head->next;
		free(buf->value);
		free(buf);
	}
}

void list_print(Node *head)
{
	while (head)
	{
		printf("%s\n", head->value);
		head = head->next;
	}
}