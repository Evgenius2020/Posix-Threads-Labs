#include <pthread.h>

#ifndef RWLOCK
#include "../error_check_mutex.c"
pthread_mutex_t list_access_mutex;
void list_access_init()
{
    list_access_mutex = errorcheck_mutex_try_init();
    printf("MUTEX implementation initilalized!\n");
}
void list_access_readlock()
{
    errorcheck_mutex_try_lock(&list_access_mutex);
}
void list_access_writelock()
{
    errorcheck_mutex_try_lock(&list_access_mutex);
}
void list_access_unlock()
{
    errorcheck_mutex_try_unlock(&list_access_mutex);
}
void list_access_destroy()
{
    errorcheck_mutex_try_destroy(&list_access_mutex);
}
#else
pthread_rwlock_t list_access_rwlock;
#include "../rwlock.c"
void list_access_init()
{
    rwlock_try_init(&list_access_rwlock);
    printf("RWLOCK implementation initilalized!\n");
}
void list_access_readlock()
{
    rwlock_try_rdlock(&list_access_rwlock);
}
void list_access_writelock()
{
    rwlock_try_wrlock(&list_access_rwlock);
}
void list_access_unlock()
{
    rwlock_try_unlock(&list_access_rwlock);
}
void list_access_destroy()
{
    rwlock_try_destroy(&list_access_rwlock);
}
#endif