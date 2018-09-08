#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *print_with_delay(void *times)
{
    unsigned i;
    unsigned print_times = *(unsigned*)times;

    for (i = 0; i < print_times; i++)
    {
        printf("Thread: %d!\n", i + 1);
        sleep(1);
    }

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    pthread_t thread;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [strings_number]\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    unsigned print_times = atoi(argv[1]);

    // int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    //                         void *(*start_routine) (void *), void *arg);

    // Error can occur by excessing of the per-process thread limit.
    // Command to check limit:
    // cat /proc/sys/kernel/threads-max 
    if (0 != pthread_create(&thread, NULL, print_with_delay, (void *)&print_times))
    {
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }
    printf("Process: created a thread!\n");
    
    if (0 != pthread_join(thread, NULL))
    {
        perror("Failed to join thread");
        exit(EXIT_FAILURE);
    }
    printf("Process: joined a thread!\n");

    exit(EXIT_SUCCESS);
}