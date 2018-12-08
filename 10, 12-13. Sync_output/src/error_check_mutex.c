#include <stdlib.h>

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

pthread_mutex_t init_mutex()
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t attrs;
    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init(&mutex, &attrs);

    return mutex;
}

void try_destroy_mutex(pthread_mutex_t *mutex)
{
    if (0 != pthread_mutex_destroy(mutex))
    {
        fprintf(stderr, "%s\nFailed to destroy mutex\n", ERROR_COLOR);
        perror("");
        exit(EXIT_FAILURE);
    }
}