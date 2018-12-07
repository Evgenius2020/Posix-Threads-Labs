#ifndef SHARED
#define SHARED

#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT_TIMES 5

#define SEM_PARENT_LINK "parent"
#define SEM_CHILD_LINK "child"

char *CHILD_COLOR = "\x1b[35;1m";
char *PARENT_COLOR = "\x1b[36;1m";
char *ERROR_COLOR = "\x1b[31;1m";

void open_semaphores(sem_t *parent, sem_t *child)
{
	parent = sem_open(SEM_PARENT_LINK, 0);
	child = sem_open(SEM_CHILD_LINK, 0);

	if (parent == SEM_FAILED || child == SEM_FAILED)
	{
		fprintf(stderr, "%s\nFailed to sem_open\n", ERROR_COLOR);
		perror("");
		exit(EXIT_FAILURE);
	}
}

#endif