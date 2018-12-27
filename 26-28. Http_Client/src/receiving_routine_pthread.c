#include "lib/cond.h"
#include "lib/console_app_tools.h"
#include "lib/mutex.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 8192
#define LINES_LEFT_DEFAULT 25

typedef struct
{
    int socketfd;
    char *buffer;
    int *buffer_bytes_count;
    int *is_socketfd_eof;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
} Socket_Routine_Args;

void *socket_routine(void *args_raw)
{
    Socket_Routine_Args *args = (Socket_Routine_Args *)args_raw;

    while (!*args->is_socketfd_eof)
    {
        mutex_try_lock(args->mutex);
        while (*args->buffer_bytes_count)
            cond_try_wait(args->cond, args->mutex);

        int bytes_read = read(args->socketfd, args->buffer, BUFSIZE);
        if (-1 == bytes_read)
            throw_and_exit("read");
        else if (bytes_read == 0)
            *args->is_socketfd_eof = 1;
        else
            *args->buffer_bytes_count = bytes_read;

        cond_try_signal(args->cond);
        mutex_try_unlock(args->mutex);
    }

    pthread_exit(EXIT_SUCCESS);
}

void receiving_routine(int socketfd)
{
    pthread_mutex_t mutex;
    mutex_try_init(&mutex);
    pthread_cond_t cond;
    cond_try_init(&cond);
    char buffer[BUFSIZE];
    int buffer_bytes_count = 0;
    int is_socketfd_eof = 0;
    int lines_left = LINES_LEFT_DEFAULT;

    Socket_Routine_Args args;
    args.socketfd = socketfd;
    args.buffer = buffer;
    args.buffer_bytes_count = &buffer_bytes_count;
    args.is_socketfd_eof = &is_socketfd_eof;
    args.mutex = &mutex;
    args.cond = &cond;

    pthread_t socket_thread;
    if (pthread_create(&socket_thread, NULL, socket_routine, &args))
        throw_and_exit("pthread_create");

    while (!is_socketfd_eof)
    {
        mutex_try_lock(&mutex);
        while (!buffer_bytes_count && !is_socketfd_eof)
            cond_try_wait(&cond, &mutex);

        while (buffer_bytes_count)
        {
            int pos;
            for (pos = 0; pos < buffer_bytes_count; pos++)
                if (buffer[pos] == '\n')
                {
                    lines_left--;
                    if (!lines_left)
                    {
                        pos++;
                        break;
                    }
                }

            if (write(STDOUT_FILENO, buffer, pos) < pos)
                throw_and_exit("write");
            else
                buffer_bytes_count -= pos;
            if (buffer_bytes_count)
                memmove(buffer, buffer + pos, buffer_bytes_count);

            if (!lines_left)
            {
                printf("%s\nPress enter to scroll down.%s\n", YELLOW_COLOR, WHITE_COLOR);
                while (getchar() != '\n')
                    ;
                lines_left = LINES_LEFT_DEFAULT;
            }
        }

        cond_try_signal(&cond);
        mutex_try_unlock(&mutex);
    }

    if (pthread_join(socket_thread, NULL))
        throw_and_exit("pthread_join");
}
