#include "console_colors.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_rwlock_t rwlock_try_init()
{
    pthread_rwlock_t rwlock;
    if (0 != pthread_rwlock_init(&rwlock, NULL))
    {
        fprintf(stderr, "%s\nFailed to init rwlock\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }

    return rwlock;
}

void rwlock_try_rdlock(pthread_rwlock_t *rwlock)
{
    if (0 != pthread_rwlock_rdlock(rwlock))
    {
        fprintf(stderr, "%s\nFailed to rdlock rwlock\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}

void rwlock_try_wrlock(pthread_rwlock_t *rwlock)
{
    if (0 != pthread_rwlock_wrlock(rwlock))
    {
        fprintf(stderr, "%s\nFailed to wrlock rwlock\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}

void rwlock_try_unlock(pthread_rwlock_t *rwlock)
{
    if (0 != pthread_rwlock_unlock(rwlock))
    {
        fprintf(stderr, "%s\nFailed to unlock rwlock\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}

void rwlock_try_destroy(pthread_rwlock_t *rwlock)
{
    if (0 != pthread_rwlock_destroy(rwlock))
    {
        fprintf(stderr, "%s\nFailed to destroy rwlock\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}