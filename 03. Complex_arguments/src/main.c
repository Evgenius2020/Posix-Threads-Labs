#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    unsigned thread_number;
    unsigned strings_number;
    char **strings;
} Print_Arguments;

void *print_with_delay(void *print_arguments_raw)
{
    Print_Arguments *print_arguments = print_arguments_raw;
    unsigned argument_pos;

    for (argument_pos = 0; argument_pos < print_arguments->strings_number; argument_pos++)
    {
        printf("Thread [#%d]: %s!\n", print_arguments->thread_number,
               print_arguments->strings[argument_pos]);
        sleep(1);
    }

    pthread_exit(0);
}

int parse_arguments(int argc, char **argv, unsigned *threads_number,
                    Print_Arguments **print_arguments)
{
    *threads_number = atoi(argv[1]);
    if (0 == *threads_number)
        return -1;
    *print_arguments = malloc(*threads_number * sizeof(Print_Arguments));
    unsigned curr_thread;
    unsigned strings_number;
    unsigned argument_pos = 2;
    for (curr_thread = 0; curr_thread < *threads_number; curr_thread++)
    {
        strings_number = atoi(argv[argument_pos]);
        if (0 == strings_number)
        {
            free(*print_arguments);
            return -1;
        }
        (*print_arguments)[curr_thread].thread_number = curr_thread;
        (*print_arguments)[curr_thread].strings_number = strings_number;
        (*print_arguments)[curr_thread].strings = &argv[argument_pos + 1];
        argument_pos += strings_number + 1;
        if (argc < argument_pos)
        {
            free(*print_arguments);
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr,
                "Usage: %s [threads_number > 0] [[[strings_number > 0] [[string] ..]] ..]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned threads_number;
    Print_Arguments *print_arguments;
    if (0 != parse_arguments(argc, argv, &threads_number, &print_arguments))
    {
        fprintf(stderr, "Process: Format error\n");
        exit(EXIT_FAILURE);
    }

    pthread_t *threads = malloc(threads_number * sizeof(pthread_t));
    unsigned argument_pos;

    for (argument_pos = 0; argument_pos < threads_number; argument_pos++)
        if (0 != pthread_create(&threads[argument_pos], NULL,
                                print_with_delay, &print_arguments[argument_pos]))
            fprintf(stderr, "Process: Failed to create thread [#%d]\n", argument_pos);
        else
            printf("Process: created a thread [#%d]\n", argument_pos);

    for (argument_pos = 0; argument_pos < threads_number; argument_pos++)
        if (0 != pthread_join(threads[argument_pos], NULL))
            fprintf(stderr, "Process: Failed to join thread [#%d]\n", argument_pos);
        else
            printf("Process: joined a thread [#%d]\n", argument_pos);

    free(print_arguments);
    free(threads);
    exit(EXIT_SUCCESS);
}