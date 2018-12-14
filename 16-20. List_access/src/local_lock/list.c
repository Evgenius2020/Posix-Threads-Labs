#include "list.h"
#include "../console_colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COMPARE_PROCESSING_DELAY 1

Node *list_add_node(Node *head, char *value)
{
	Node *new_node = (Node *)malloc(sizeof(Node));
	new_node->value = value;
	new_node->next = head;
	if (head)
	{
		list_access_readlock(&head->list_access);
		new_node->next = head;
		list_access_unlock(&head->list_access);
	}
	else
		new_node->next = head;

	return new_node;
}

int list_cmp(Node *left, Node *right)
{
	list_access_readlock(&left->list_access);
	list_access_readlock(&right->list_access);

	printf("%s[Child] Comparing '%s' with '%s'...%s\n",
		   CHILD_COLOR, left->value, right->value, NORMAL_COLOR);
	sleep(COMPARE_PROCESSING_DELAY);
	int result = strcmp(left->value, right->value);

	list_access_unlock(&left->list_access);
	list_access_unlock(&right->list_access);

	return result;
}

void list_swap(Node *prev, Node *curr, Node *next)
{
	if (prev)
	{
		list_access_writelock(&prev->list_access);
		prev->next = next;
		list_access_unlock(&prev->list_access);
	}

	list_access_writelock(&curr->list_access);
	list_access_writelock(&next->list_access);

	curr->next = next->next;
	next->next = curr;

	list_access_unlock(&curr->list_access);
	list_access_unlock(&next->list_access);

	printf("%s[Child] Swapped '%s' with '%s'!%s\n",
		   CHILD_COLOR, curr->value, next->value, NORMAL_COLOR);
}

void list_destroy(Node *head)
{
	Node *buf;
	while (head)
	{
		buf = head;
		head = head->next;
		list_access_destroy(&head->list_access);
		free(buf->value);
		free(buf);
	}
}

void list_print(Node *head)
{
	while (head)
	{
		list_access_readlock(&head->list_access);
		printf("%s\n", head->value);
		list_access_unlock(&head->list_access);
		head = head->next;
	}
}