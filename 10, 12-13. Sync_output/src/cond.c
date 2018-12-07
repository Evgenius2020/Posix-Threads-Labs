#include "console_colors.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool is_child_turn;

void initialize_sync_primitives()
{
    is_child_turn = false;
}
void parent_lock()
{
    pthread_mutex_lock(&mutex);
    while (is_child_turn)
        pthread_cond_wait(&cond, &mutex);
}

void parent_unlock()
{
    is_child_turn = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void child_lock()
{
    pthread_mutex_lock(&mutex);
    while (!is_child_turn)
        pthread_cond_wait(&cond, &mutex);
}

void child_unlock()
{
    is_child_turn = false;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void dispose_sync_primitives()
{
    if (0 != pthread_mutex_destroy(&mutex))
    {
        fprintf(stderr, "%s\nFailed to destroy mutex\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}