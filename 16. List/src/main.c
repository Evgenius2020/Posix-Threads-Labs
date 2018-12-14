#include "error_check_mutex.c"
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_SIZE 80
#define SORT_THREAD_SLEEP_INTERVAL 5

pthread_mutex_t list_access_mutex;
node *list_head;

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

		errorcheck_mutex_try_lock(&list_access_mutex);

		if (line[0] == '\n')
		{
			printf("%s[Parent] Printing list...\n", PARENT_COLOR);
			list_print(list_head);
		}
		else
		{
			list_head = list_add_node(list_head, line);
			printf("%s[Parent] Inserted into list value %s", PARENT_COLOR, line);
		}

		errorcheck_mutex_try_unlock(&list_access_mutex);
	}
}

void *sort_list()
{
	while (1)
	{
		sleep(SORT_THREAD_SLEEP_INTERVAL);

		printf("%s[Child] Sorting list...\n", CHILD_COLOR);
		errorcheck_mutex_try_lock(&list_access_mutex);

		node *i, *j;
		char *buf;
		for (i = list_head; i; i = i->next)
			for (j = i->next; j; j = j->next)
				if (strcmp(i->value, j->value) > 0)
				{
					buf = i->value;
					i->value = j->value;
					j->value = buf;
				}

		errorcheck_mutex_try_unlock(&list_access_mutex);
		printf("%s[Child] Sorting list complete!%s\n", CHILD_COLOR, NORMAL_COLOR);
	}
}

void destroy_list()
{
	list_destroy(list_head);
	exit(EXIT_SUCCESS);
}

int main()
{
	list_head = NULL;
	list_access_mutex = errorcheck_mutex_init();

	pthread_t thread;
	atexit(destroy_list);
	if (0 != pthread_create(&thread, NULL, sort_list, NULL))
	{
		fprintf(stderr, "%s[Parent] Failed to create thread!\n", ERROR_COLOR);
		perror("");
		exit(EXIT_FAILURE);
	}

	start_strings_appending_routine();
}