#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *CHILD_COLOR = "\x1b[35;1m";
char *PARENT_COLOR = "\x1b[36;1m";
char *ERROR_COLOR = "\x1b[31;1m";

typedef struct
{
    unsigned threads_n;
    unsigned thread_id;
    int limit;
    double result;
    pthread_t thread;
} Calc_Pi_Args;

int terminate_flag = 0;
pthread_mutex_t mutex;

void *calulate_pi_row(void *Calc_Pi_Args_raw)
{
    double result = 0;
    Calc_Pi_Args *args = Calc_Pi_Args_raw;

    printf("%s[Child#%u] Started a sum routine..\n",
           CHILD_COLOR, args->thread_id);
    for (int n = args->thread_id;
         (args->limit == -1) || (n < args->limit);
         n += args->threads_n)
    {
        result += 1.0 / (n * 4.0 + 1.0);
        result -= 1.0 / (n * 4.0 + 3.0);

        if ((n / 10000) % 2)
        {
            pthread_mutex_lock(&mutex);
            int termimate_flag_copy = terminate_flag;
            pthread_mutex_unlock(&mutex);
            if (termimate_flag_copy)
                break;
        }
    }
    printf("%s[Child#%u] Returning sum (%e)..\n",
           CHILD_COLOR, args->thread_id, result);

    args->result = result;
    pthread_exit(args);
}

void on_sigint_received(int arg)
{
    pthread_mutex_lock(&mutex);
    terminate_flag = 1;
    printf("%s[Parent] received SIGINT!\n", PARENT_COLOR);
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[])
{
    if ((argc < 2) || (argc > 3))
    {
        fprintf(stderr, "%sUsage: %s [treads] [row_members=-1(infinity)].\n",
                ERROR_COLOR, argv[0]);
        exit(EXIT_FAILURE);
    }
    int threads_n = atoi(argv[1]);
    if (threads_n < 0)
    {
        fprintf(stderr, "%sNumber of threads must be greater than 0!\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }
    int limit = -1;
    if (argc == 3)
    {
        limit = atoi(argv[2]);
        if (limit < 0)
        {
            fprintf(stderr, "%sNumber of row numbers must be non-negative!\n", ERROR_COLOR);
            exit(EXIT_FAILURE);
        }
    }
    if (SIG_ERR == signal(SIGINT, on_sigint_received))
    {
        fprintf(stderr, "%sFailed to set SIGINT handler!\n", ERROR_COLOR);
        exit(EXIT_FAILURE);
    }

    if (0 != pthread_mutex_init(&mutex, NULL))
    {
        fprintf(stderr, "%sFailed to create mutex!\n", ERROR_COLOR);
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

    pthread_mutex_destroy(&mutex);
    free(pi_args);
    exit(EXIT_SUCCESS);
}