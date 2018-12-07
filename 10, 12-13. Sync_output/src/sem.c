#include "console_colors.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t parent;
sem_t child;

void initialize_sync_primitives()
{
    sem_init(&parent, 0, 1);
    sem_init(&child, 0, 1);
    sem_wait(&parent);
    sem_wait(&child);
}

void parent_lock()
{
    sem_wait(&parent);
}

void parent_unlock()
{
    sem_post(&child);
}

void child_lock()
{
    sem_wait(&child);
}

void child_unlock()
{
    sem_post(&parent);
}

void dispose_sync_primitives()
{
    if (0 != sem_destroy(&parent))
    {
        fprintf(stderr, "%s\nFailed to destroy parent-semaphore\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }

    if (0 != sem_destroy(&child))
    {
        fprintf(stderr, "%s\nFailed to destroy child-semaphore\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}