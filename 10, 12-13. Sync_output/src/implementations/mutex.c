#include "../console_colors.h"
#include "../error_check_mutex.c"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

void initialize_sync_primitives()
{
    mutex1 = errorcheck_mutex_init();
    mutex2 = errorcheck_mutex_init();
    mutex3 = errorcheck_mutex_init();
}

void parent_prelock()
{
    errorcheck_mutex_try_lock(&mutex1);
    errorcheck_mutex_try_lock(&mutex2);
}

void child_prelock()
{
    errorcheck_mutex_try_lock(&mutex3);
}

void child_postunlock()
{
    errorcheck_mutex_try_unlock(&mutex3);
}

void parent_postunlock()
{
    errorcheck_mutex_try_unlock(&mutex1);
    errorcheck_mutex_try_unlock(&mutex2);
}

void child_lock()
{
    errorcheck_mutex_try_lock(&mutex1);
    errorcheck_mutex_try_unlock(&mutex3);
    errorcheck_mutex_try_lock(&mutex2);
}

void child_unlock()
{
    errorcheck_mutex_try_unlock(&mutex1);
    errorcheck_mutex_try_lock(&mutex3);
    errorcheck_mutex_try_unlock(&mutex2);
}

void parent_lock()
{
    errorcheck_mutex_try_lock(&mutex1);
    errorcheck_mutex_try_unlock(&mutex3);
    errorcheck_mutex_try_lock(&mutex2);
}

void parent_unlock()
{
    errorcheck_mutex_try_unlock(&mutex1);
    errorcheck_mutex_try_lock(&mutex3);
    errorcheck_mutex_try_unlock(&mutex2);
}

void dispose_sync_primitives()
{
    errorcheck_mutex_try_destroy(&mutex1);
    errorcheck_mutex_try_destroy(&mutex2);
    errorcheck_mutex_try_destroy(&mutex3);
}