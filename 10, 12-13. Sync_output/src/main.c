#include "console_colors.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PRINT_TIMES 4

#if defined MUTEX
#include "mutex.c"
#elif defined COND
#include "cond.c"
#elif defined SEM
#include "sem.c"
#endif

void initialize_sync_primitives();

void parent_prelock();
void parent_lock();
void parent_unlock();
void parent_postunlock();

void child_prelock();
void child_lock();
void child_unlock();
void child_postunlock();

void dispose_sync_primitives();

void *print_child(void *arg)
{
	child_prelock();

	for (int j = 0; j < PRINT_TIMES; j++)
	{
		child_lock();

		printf("%s[Child]: ", CHILD_COLOR);
		for (int i = 0; i < 10; i++)
		{
			printf("%c", 'A' + i);
			fflush(stdout);
			usleep(50000);
		}
		printf("\n");

		child_unlock();
	}

	child_postunlock();
	pthread_exit(0);
}

void print_parent()
{
	for (int j = 0; j < PRINT_TIMES; j++)
	{
		printf("%s[Parent]: ", PARENT_COLOR);
		for (int i = 0; i < 10; i++)
		{
			printf("%c", '0' + i);
			fflush(stdout);
			usleep(50000);
		}
		printf("\n");

		parent_unlock();
		parent_lock();
	}

	parent_postunlock();
}

int main()
{
	initialize_sync_primitives();
	parent_prelock();

	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, print_child, NULL))
	{
		fprintf(stderr, "%s[Parent] Failed to create thread!\n", ERROR_COLOR);
		exit(EXIT_FAILURE);
	}

	print_parent();
	if (0 != pthread_join(thread, NULL))
	{
		fprintf(stderr, "%s[Parent] Failed to join thread\n", ERROR_COLOR);
		exit(EXIT_FAILURE);
	}

	dispose_sync_primitives();
	exit(EXIT_SUCCESS);
}