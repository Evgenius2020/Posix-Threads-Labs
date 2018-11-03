#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *CHILD_COLOR = "\x1b[35m";
char *PARENT_COLOR = "\x1b[36;1m";
char *ERROR_COLOR = "\x1b[31m";

typedef struct
{
    unsigned threads_n;
    unsigned thread_id;
    int limit;
    double result;
    pthread_t thread;
} Calc_Pi_Args;

void *calulate_pi_row(void *Calc_Pi_Args_raw)
{
    double result = 0;
    Calc_Pi_Args *args = Calc_Pi_Args_raw;

    printf("%s[Child#%u] Started a sum routine for ..\n",
           CHILD_COLOR, args->thread_id);
    for (unsigned n = args->thread_id;
         (n < args->limit) || (args->limit == -1);
         n += args->threads_n)
    {
        result += 1.0 / (n * 4.0 + 1.0);
        result -= 1.0 / (n * 4.0 + 3.0);
    }
    printf("%s[Child#%u] Returning sum (%e)..\n",
           CHILD_COLOR, args->thread_id, result);

    args->result = result;
    pthread_exit(args);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "%sUsage: %s [treads] [row_members=infinity].\n",
                ERROR_COLOR, argv[0]);
        exit(EXIT_FAILURE);
    }
    int threads_n = atoi(argv[1]);
    if (threads_n < 0)
    {
        fprintf(stderr, "%sNumber of threads must be greater than 0!\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }
    int limit = atoi(argv[2]);
    if (limit < 0)
    {
        fprintf(stderr, "%sNumber of row numbers must be non-negative!\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }

    Calc_Pi_Args **pi_args = malloc(sizeof(Calc_Pi_Args *) * threads_n);
    for (unsigned i = 0; i < threads_n; i++)
    {
        pi_args[i] = malloc(sizeof(Calc_Pi_Args));
        pi_args[i]->limit = limit;
        pi_args[i]->thread_id = i;
        pi_args[i]->threads_n = threads_n;
        if (0 !=
            pthread_create(&pi_args[i]->thread, NULL, calulate_pi_row, (void *)pi_args[i]))
        {
            fprintf(stderr, "%s[Parent] Failed to create thread!\n", ERROR_COLOR);
            perror("");
            exit(EXIT_FAILURE);
        }
        printf("%s[Parent] Created a Child#%d.\n",
               PARENT_COLOR, pi_args[i]->thread_id);
    }

    Calc_Pi_Args *ret_args;
    double result = 0;
    for (unsigned i = 0; i < threads_n; i++)
    {
        if (0 != pthread_join(pi_args[i]->thread, (void **)&ret_args))
        {
            fprintf(stderr, "%s[Parent] Failed to join thread\n", ERROR_COLOR);
            perror("");
            exit(EXIT_FAILURE);
        }
        printf("%s[Parent] Joined a Child#%d.\n",
               PARENT_COLOR, ret_args->thread_id);
        result += ret_args->result;
        free(pi_args[i]);
    }

    result *= 4;
    printf("%s[Parent] Pi=%e.\n", PARENT_COLOR, result);

    free(pi_args);
    exit(EXIT_SUCCESS);
}