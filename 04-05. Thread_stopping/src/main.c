#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

char *CHILD_COLOR = "\x1b[35m";
char *PARENT_COLOR = "\x1b[36;1m";
char *ERROR_COLOR = "\x1b[31m";

void on_thread_stop(void *arg)
{
    printf("%sStopped by process!\n", CHILD_COLOR);
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
    // from thread, but not by 'return').q
    pthread_cleanup_push(on_thread_stop, NULL);

    printf("%sChild: Print routine started!\n", CHILD_COLOR);
    for (i = 0; i < print_times; i++)
    {
        printf("%sChild: %d!\n", CHILD_COLOR, i + 1);
        sleep(1);
    }
    printf("%sChild Print routine complete!\n", CHILD_COLOR);

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
                "%sUsage: %s [seconds_to_print > 0] [seconds_before_stop > 0]\n",
                ERROR_COLOR, argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned print_times = atoi(argv[1]);
    unsigned seconds_before_stop = atoi(argv[2]);
    if (!(print_times && seconds_before_stop))
    {
        fprintf(stderr, "%sFormat error\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }

    if (0 != pthread_create(&thread, NULL, print_with_delay, (void *)&print_times))
    {
        fprintf(stderr, "%sProcess: Failed to create thread\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }
    printf("%sMain: Created a thread!\n", PARENT_COLOR);

    while (seconds_before_stop)
    {
        printf("%sMain: %d seconds before stopping thread\n",
               PARENT_COLOR, seconds_before_stop--);
        sleep(1);
    }

    if (0 != pthread_cancel(thread))
    {
        fprintf(stderr, "%sProcess: Failed to cancel thread\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }
    printf("%sMain: Stoped a thread!\n", PARENT_COLOR);

    // You need to wait thread cancelation. Othewrwise 'on_thread_stop' will
    // not work because of the program termination.
    // Also you can use 'sleep(1)' at the process, without join.
    if (0 != pthread_join(thread, NULL))
    {
        fprintf(stderr, "%sMain: Failed to join thread\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }
    printf("%sMain: Joined a thread!\n", PARENT_COLOR);

    exit(EXIT_SUCCESS);
}