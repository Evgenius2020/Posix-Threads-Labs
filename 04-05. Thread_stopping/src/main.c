#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void on_thread_stop(void *arg)
{
    printf("Thread: Stopped by process!\n");
}

void *print_with_delay(void *print_times_raw)
{
    unsigned i;
    unsigned print_times = *(unsigned *)print_times_raw;

    // Creating a handler for stopping thread by process.
    //
    // 'pthread_cleanup_push' it's a macros. So you need to 
    // call 'pthread_cleanup_pop' necessarily at same block.
    //
    // These functions interact with the stack of functions, 
    // that are executed at thread stopping (by proccess or
    // from thread, but not by 'return').
    pthread_cleanup_push(on_thread_stop, NULL);

    printf("Thread: Print routine started!\n");
    for (i = 0; i < print_times; i++)
    {
        printf("Thread: %d!\n", i + 1);
        sleep(1);
    }
    printf("Thread: Print routine complete!\n");

    // Extracting all functions from stack. Argument is a bool-typed.
    // If 'true', all popped functions will be executed.
    pthread_cleanup_pop(0);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    pthread_t thread;

    if (argc < 3)
    {
        fprintf(stderr, 
            "Usage: %s [seconds_to_print > 0] [seconds_before_stop > 0]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned print_times = atoi(argv[1]);
    unsigned seconds_before_stop = atoi(argv[2]);
    if (!(print_times && seconds_before_stop))
    {
        fprintf(stderr, "Format error\n");
        exit(EXIT_FAILURE);
    }

    if (0 != pthread_create(&thread, NULL, print_with_delay, (void *)&print_times))
    {
        perror("Process: Failed to create thread");
        exit(EXIT_FAILURE);
    }
    printf("Process: created a thread!\n");

    while (seconds_before_stop)
    {
        printf("Process: %d seconds before stopping thread\n", seconds_before_stop--);
        sleep(1);
    }

    if (0 != pthread_cancel(thread))
    {
        perror("Process: Failed to stop thread");
        exit(EXIT_FAILURE);
    }
    printf("Process: Stopped a thread\n");

    // You need to wait thread cancelation. Othewrwise 'on_thread_stop' will 
    // not work because of the program termination.
    // Also you can use 'sleep(1)' at the process, without join.
    if (0 != pthread_join(thread, NULL))
    {
        perror("Failed to join thread");
        exit(EXIT_FAILURE);
    }
    printf("Process: Joined a thread!\n");

    exit(EXIT_SUCCESS);
}