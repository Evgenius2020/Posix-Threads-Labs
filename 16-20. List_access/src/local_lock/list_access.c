#include <pthread.h>

#ifndef RWLOCK
#include "../error_check_mutex.c"
typedef pthread_mutex_t list_access_type;
pthread_mutex_t list_access_init()
{
    return errorcheck_mutex_try_init();
}
void list_access_readlock(pthread_mutex_t *mutex)
{
    errorcheck_mutex_try_lock(mutex);
}
void list_access_writelock(pthread_mutex_t *mutex)
{
    errorcheck_mutex_try_lock(mutex);
}
void list_access_unlock(pthread_mutex_t *mutex)
{
    errorcheck_mutex_try_unlock(mutex);
}
void list_access_destroy(pthread_mutex_t *mutex)
{
    errorcheck_mutex_try_destroy(mutex);
}
#else
#include "../rwlock.c"
typedef pthread_rwlock_t list_access_type;

pthread_rwlock_t list_access_init()
{
    return rwlock_try_init();
}
void list_access_readlock(pthread_rwlock_t* rwlock)
{
    rwlock_try_rdlock(rwlock);
}
void list_access_writelock(pthread_rwlock_t* rwlock)
{
    rwlock_try_wrlock(rwlock);
}
void list_access_unlock(pthread_rwlock_t* rwlock)
{
    rwlock_try_unlock(rwlock);
}
void list_access_destroy(pthread_rwlock_t* rwlock)
{
    rwlock_try_destroy(rwlock);
}
#endif