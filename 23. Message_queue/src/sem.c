#include "console_colors.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

void throw_and_exit(char *call_name)
{
    fprintf(stderr, "%s\nFailed to %s\n", ERROR_COLOR, call_name);
    perror("");
    exit(EXIT_FAILURE);
}

void sem_try_init(sem_t *sem, int shared, int initial_value)
{
    if (sem_init(sem, shared, initial_value))
        throw_and_exit("sem_init");
}

void sem_try_post(sem_t *sem)
{
    if (sem_post(sem))
        throw_and_exit("sem_post");
}

void sem_try_wait(sem_t *sem)
{
    if (sem_wait(sem))
        throw_and_exit("sem_wait");
}

void sem_try_destroy(sem_t *sem)
{
    if (sem_destroy(sem))
        throw_and_exit("sem_close");
}