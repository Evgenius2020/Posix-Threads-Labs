#include "console_colors.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

void try_lock_mutex(pthread_mutex_t *mutex)
{
    if (0 != pthread_mutex_lock(mutex))
    {
        fprintf(stderr, "%s\nFailed to lock mutex\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }
}

void try_unlock_mutex(pthread_mutex_t *mutex)
{
    if (0 != pthread_mutex_unlock(mutex))
    {
        fprintf(stderr, "%s\nFailed to unlock mutex\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }
}

void initialize_sync_primitives()
{
    try_lock_mutex(&mutex1);
    try_lock_mutex(&mutex2);
}

void child_lock()
{
    try_lock_mutex(&mutex1);
    try_unlock_mutex(&mutex2);
    try_lock_mutex(&mutex3);
}

void child_unlock()
{
    try_unlock_mutex(&mutex1);
    try_lock_mutex(&mutex2);
    try_unlock_mutex(&mutex3);
}

void parent_lock()
{
    try_lock_mutex(&mutex1);
    try_unlock_mutex(&mutex2);
    try_lock_mutex(&mutex3);
}

void parent_unlock()
{
    try_unlock_mutex(&mutex1);
    try_lock_mutex(&mutex2);
    try_unlock_mutex(&mutex3);
}

void dispose_sync_primitives()
{
    // if ((0 != pthread_mutex_destroy(&mutex1)) ||
    //     (0 != pthread_mutex_destroy(&mutex2)) ||
    //     (0 != pthread_mutex_destroy(&mutex3)))
    // {
    //     fprintf(stderr, "%s\nFailed to destroy mutex\n", ERROR_COLOR);
    //     perror("");
    //     exit(EXIT_FAILURE);
    // }
}