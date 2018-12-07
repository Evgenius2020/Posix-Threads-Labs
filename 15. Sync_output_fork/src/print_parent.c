#include "shared.h"
#include <unistd.h>

void print_parent()
{
	sem_t* parent;
	sem_t* child;
	open_semaphores(&parent, &child);

	for (int j = 0; j < PRINT_TIMES; j++)
	{
		sem_wait(parent);

		printf("%s[Parent]: ", PARENT_COLOR);
		for (int i = 0; i < 10; i++)
		{
			printf("%c", '0' + i);
			fflush(stdout);
			usleep(50000);
		}
		printf("\n");

		sem_post(child);
	}
}