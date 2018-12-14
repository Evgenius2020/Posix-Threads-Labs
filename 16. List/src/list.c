#include "list.h"
#include <stdio.h>
#include <stdlib.h>

node *list_add_node(node *last, char *value)
{
	node *new_node = (node *)malloc(sizeof(node));
	new_node->value = value;
	new_node->next = last;
	return new_node;
}

void list_destroy(node *head)
{
	node *buf;
	while (head)
	{
		buf = head;
		head = head->next;
		free(buf->value);
		free(buf);
	}
}

void list_print(node *head)
{
	while (head)
	{
		printf("%s", head->value);
		head = head->next;
	}
}