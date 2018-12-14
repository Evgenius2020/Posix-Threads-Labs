#include "../console_colors.h"
#include "list.c"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_SIZE 80
#define SORT_THREAD_SLEEP_INTERVAL 5
#define SORT_THREADS_NUMBER 1

Node *list_head = NULL;

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
			printf("%s[Parent] Printing list...\n", PARENT_COLOR);
			list_print(list_head);
			printf("%s[Parent] Printing list complete!%s\n", PARENT_COLOR, NORMAL_COLOR);
		}
		else
		{
			line[strlen(line) - 1] = '\0';
			list_head = list_add_node(list_head, line);
			printf("%s[Parent] Inserted '%s'%s\n", PARENT_COLOR, line, NORMAL_COLOR);
		}
	}
}

void *sort_list()
{
	while (1)
	{
		sleep(SORT_THREAD_SLEEP_INTERVAL);
		if (!list_head)
		{
			continue;
		}

		printf("%s[Child] Sorting list...\n", CHILD_COLOR);

		for (Node *temp_head = list_head; temp_head; temp_head = temp_head->next)
		{
			Node *prev = NULL;
			Node *curr = temp_head;
			Node *next = temp_head->next;

			while (next)
			{
				if (list_cmp(curr, next) > 0)
				{
					if (curr == list_head) 
						list_head = next;
					if (curr == temp_head) 
						temp_head = next;
					list_swap(prev, curr, next);
					curr = next;
				}
				prev = curr;
				curr = curr->next;
				next = curr->next;
			}
		}

		printf("%s[Child] Sorting list complete!%s\n", CHILD_COLOR, NORMAL_COLOR);
	}
}

int main()
{
	pthread_t sort_threads[SORT_THREADS_NUMBER];
	for (unsigned i = 0; i < SORT_THREADS_NUMBER; i++)
	{
		if (0 != pthread_create(&sort_threads[i], NULL, sort_list, NULL))
		{
			fprintf(stderr, "%s[Parent] Failed to create thread!\n", ERROR_COLOR);
			perror("");
			exit(EXIT_FAILURE);
		}
	}

	start_strings_appending_routine();
	for (unsigned i = 0; i < SORT_THREADS_NUMBER; i++)
	{
		if (0 != pthread_cancel(sort_threads[i]))
		{
			fprintf(stderr, "%s[Parent] Failed to cancel thread\n", ERROR_COLOR);
			exit(EXIT_FAILURE);
		}
		printf("%s[Parent] Stoped a thread!\n", PARENT_COLOR);
		if (0 != pthread_join(sort_threads[i], NULL))
		{
			fprintf(stderr, "%s[Parent] Failed to join thread\n", ERROR_COLOR);
			exit(EXIT_FAILURE);
		}
	}

	printf("%s[Parent] Joined a thread!\n", PARENT_COLOR);

	exit(EXIT_SUCCESS);
}