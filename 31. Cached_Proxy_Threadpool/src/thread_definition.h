#ifndef THREAD_DEFINITION
#define THREAD_DEFINITION

#include "hashmap.h"
#include "proxy/connection.h"
#include <stdlib.h>

typedef struct
{
    Connection *connections;
    size_t connections_length;
    pthread_mutex_t connections_mutex; // Local
    pthread_cond_t connections_cond; // locks thread if no connections
    Hashmap *cache;
    pthread_mutex_t *cache_mutex; // Threadpool-global
} Thread_Definition;

#endif