#include "console_colors.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t errorcheck_mutex_try_init()
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t attrs;
    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK_NP);
    if (0 != pthread_mutex_init(&mutex, &attrs))
    {
        fprintf(stderr, "%s\nFailed to init mutex\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }

    return mutex;
}

void errorcheck_mutex_try_lock(pthread_mutex_t *mutex)
{
    if (0 != pthread_mutex_lock(mutex))
    {
        fprintf(stderr, "%s\nFailed to lock mutex\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}

void errorcheck_mutex_try_unlock(pthread_mutex_t *mutex)
{
    if (0 != pthread_mutex_unlock(mutex))
    {
        fprintf(stderr, "%s\nFailed to unlock mutex\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}

void errorcheck_mutex_try_destroy(pthread_mutex_t *mutex)
{
    if (0 != pthread_mutex_destroy(mutex))
    {
        fprintf(stderr, "%s\nFailed to destroy mutex\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}