#include "../console_colors.h"
#ifndef LOCAL_LOCK
#include "list.h"
#include "list_access.c"
#include "list.c"
#endif
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_SIZE 80
#define SORT_THREAD_SLEEP_INTERVAL 5

Node *list_head;

void start_strings_appending_routine()
{
	while (1)
	{
		char *line = malloc(MAX_LINE_SIZE);
		line = fgets(line, MAX_LINE_SIZE, stdin);
		if (line == NULL)
		{
			fprintf(stderr, "%s[Parent] Failed read string!\n", ERROR_COLOR);
			perror("");
			exit(EXIT_FAILURE);
		}
		if (strcmp(line, ".\n") == 0)
			break;

		if (line[0] == '\n')
		{
			list_access_readlock();
			printf("%s[Parent] Printing list...\n", PARENT_COLOR);
			list_print(list_head);
			printf("%s[Parent] Printing list complete!%s\n", PARENT_COLOR, NORMAL_COLOR);
		}
		else
		{
			list_access_writelock();
			line[strlen(line) - 1] = '\0';
			list_head = list_add_node(list_head, line);
			printf("%s[Parent] Inserted '%s'%s\n", PARENT_COLOR, line, NORMAL_COLOR);
		}
		list_access_unlock();
	}
}

void *sort_list()
{
	while (1)
	{
		sleep(SORT_THREAD_SLEEP_INTERVAL);

		printf("%s[Child] Sorting list...\n", CHILD_COLOR);
		list_access_writelock();

		Node *i, *j;
		char *buf;
		for (i = list_head; i; i = i->next)
			for (j = i->next; j; j = j->next)
				if (strcmp(i->value, j->value) > 0)
				{
					buf = i->value;
					i->value = j->value;
					j->value = buf;
				}

		list_access_unlock();
		printf("%s[Child] Sorting list complete!%s\n", CHILD_COLOR, NORMAL_COLOR);
	}
}

int main()
{
	list_head = NULL;
	list_access_init();

	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, sort_list, NULL))
	{
		fprintf(stderr, "%s[Parent] Failed to create thread!\n", ERROR_COLOR);
		perror("");
		exit(EXIT_FAILURE);
	}

	start_strings_appending_routine();
	if (0 != pthread_cancel(thread))
	{
		fprintf(stderr, "%s[Parent] Failed to cancel thread\n", ERROR_COLOR);
		exit(EXIT_FAILURE);
	}
	printf("%s[Parent] Stoped a thread!\n", PARENT_COLOR);
	if (0 != pthread_join(thread, NULL))
	{
		fprintf(stderr, "%s[Parent] Failed to join thread\n", ERROR_COLOR);
		exit(EXIT_FAILURE);
	}
	printf("%s[Parent] Joined a thread!\n", PARENT_COLOR);

	list_access_destroy();

	exit(EXIT_SUCCESS);
}