#include "shared.h"
#include <unistd.h>

void print_child()
{
    sem_t* parent;
    sem_t* child;
	open_semaphores(&parent, &child);

    for (int j = 0; j < PRINT_TIMES; j++)
    {
		sem_wait(child);

        printf("%s[Child]: ", CHILD_COLOR);
        for (int i = 0; i < 10; i++)
        {
            printf("%c", 'A' + i);
            fflush(stdout);
            usleep(50000);
        }
        printf("\n");

        sem_post(parent);
    }

    exit(EXIT_SUCCESS);
}