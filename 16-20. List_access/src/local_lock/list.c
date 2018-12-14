#include "list.h"
#include "../console_colors.h"
#include "../rwlock.c"
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
		rwlock_try_rdlock(&head->rwlock);
		new_node->next = head;
		rwlock_try_unlock(&head->rwlock);
	}
	else
		new_node->next = head;

	return new_node;
}

int list_cmp(Node *left, Node *right)
{
	rwlock_try_rdlock(&left->rwlock);
	rwlock_try_rdlock(&right->rwlock);

	printf("%s[Child] Comparing '%s' with '%s'...%s\n",
		   CHILD_COLOR, left->value, right->value, NORMAL_COLOR);
	sleep(COMPARE_PROCESSING_DELAY);
	int result = strcmp(left->value, right->value);

	rwlock_try_unlock(&left->rwlock);
	rwlock_try_unlock(&right->rwlock);

	return result;
}

void list_swap(Node *prev, Node *curr, Node *next)
{
	if (prev)
	{
		rwlock_try_rdlock(&prev->rwlock);
		prev->next = next;
		rwlock_try_unlock(&prev->rwlock);
	}
	
	rwlock_try_wrlock(&curr->rwlock);
	rwlock_try_wrlock(&next->rwlock);

	curr->next = next->next;
	next->next = curr;

	rwlock_try_unlock(&curr->rwlock);
	rwlock_try_unlock(&next->rwlock);

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
		rwlock_try_destroy(&head->rwlock);
		free(buf->value);
		free(buf);
	}
}

void list_print(Node *head)
{
	while (head)
	{
		rwlock_try_rdlock(&head->rwlock);
		printf("%s\n", head->value);
		rwlock_try_unlock(&head->rwlock);
		head = head->next;
	}
}