#include "print_child.c"
#include "print_parent.c"
#include "shared.h"
#include <errno.h>
#include <stdlib.h>
#include <wait.h>

void print_child();
void print_parent();

void delete_semaphores()
{
	int parent_close_status = sem_unlink(SEM_PARENT_LINK);
	int child_close_status = sem_unlink(SEM_CHILD_LINK);
	if ((0 != parent_close_status &&
		 ENOENT != errno) ||
		(0 != child_close_status &&
		 ENOENT != errno))
	{
		fprintf(stderr, "%s\nFailed to sem_unlink\n", ERROR_COLOR);
		perror("");
		exit(EXIT_FAILURE);
	}
}

void init_semaphores(sem_t *parent, sem_t *child)
{
	parent = sem_open(SEM_PARENT_LINK, O_CREAT | O_EXCL, 0666, 1);
	child = sem_open(SEM_CHILD_LINK, O_CREAT | O_EXCL, 0666, 0);

	if (parent == SEM_FAILED || child == SEM_FAILED)
	{
		fprintf(stderr, "%s\nFailed to sem_open\n", ERROR_COLOR);
		perror("");
		exit(EXIT_FAILURE);
	}
}

int main()
{
	sem_t sem_parent;
	sem_t sem_child;
	delete_semaphores();
	init_semaphores(&sem_parent, &sem_child);
	sem_close(&sem_parent);
	sem_close(&sem_child);
	atexit(delete_semaphores);

	if (0 == fork())
		print_child();

	print_parent();
	if (wait(NULL) == -1)
	{
		fprintf(stderr, "%s\nFailed to wait child process\n", ERROR_COLOR);
		perror("");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}