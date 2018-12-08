#include "console_colors.h"
#include "error_check_mutex.c"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

void initialize_sync_primitives()
{
    mutex1 = init_mutex();
    mutex2 = init_mutex();
    mutex3 = init_mutex();
}

void parent_prelock()
{
    try_lock_mutex(&mutex1);
    try_lock_mutex(&mutex2);
}

void child_prelock()
{
    try_lock_mutex(&mutex3);
}

void child_postunlock()
{
    try_unlock_mutex(&mutex3);
}

void parent_postunlock()
{
    try_unlock_mutex(&mutex1);
    try_unlock_mutex(&mutex2);
}

void child_lock()
{
    try_lock_mutex(&mutex1);
    try_unlock_mutex(&mutex3);
    try_lock_mutex(&mutex2);
}

void child_unlock()
{
    try_unlock_mutex(&mutex1);
    try_lock_mutex(&mutex3);
    try_unlock_mutex(&mutex2);
}

void parent_lock()
{
    try_lock_mutex(&mutex1);
    try_unlock_mutex(&mutex3);
    try_lock_mutex(&mutex2);
}

void parent_unlock()
{
    try_unlock_mutex(&mutex1);
    try_lock_mutex(&mutex3);
    try_unlock_mutex(&mutex2);
}

void dispose_sync_primitives()
{
    try_destroy_mutex(&mutex1);
    try_destroy_mutex(&mutex2);
    try_destroy_mutex(&mutex3);
}