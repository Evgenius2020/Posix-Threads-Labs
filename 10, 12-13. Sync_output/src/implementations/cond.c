#include "../console_colors.h"
#include "../error_check_mutex.c"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool is_child_turn;

void initialize_sync_primitives()
{
    mutex = errorcheck_mutex_init();
}

void parent_prelock()
{
    is_child_turn = false;
    errorcheck_mutex_try_lock(&mutex);
}

void parent_postunlock()
{
    errorcheck_mutex_try_unlock(&mutex);
}

void child_prelock() {}
void child_postunlock() {}

void parent_lock()
{
    errorcheck_mutex_try_lock(&mutex);
    while (is_child_turn)
        pthread_cond_wait(&cond, &mutex);
}

void parent_unlock()
{
    is_child_turn = true;
    pthread_cond_signal(&cond);
    errorcheck_mutex_try_unlock(&mutex);
}

void child_lock()
{
    errorcheck_mutex_try_lock(&mutex);
    while (!is_child_turn)
        pthread_cond_wait(&cond, &mutex);
}

void child_unlock()
{
    is_child_turn = false;
    pthread_cond_signal(&cond);
    errorcheck_mutex_try_unlock(&mutex);
}

void dispose_sync_primitives()
{
    errorcheck_mutex_try_destroy(&mutex);
}