#include "console_colors.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t sem_a, sem_b, sem_c, sem_ab;

typedef struct
{
	char *name;
	char *color;
	unsigned char delay;
	sem_t *sem_depend_1;
	sem_t *sem_depend_2;
	sem_t *sem_out;
} Worker_Definition;

void *worker_run(void *worker_definition_raw)
{
	Worker_Definition *worker_def = (Worker_Definition *)worker_definition_raw;
	sem_t *sem_depends[2] = {worker_def->sem_depend_1, worker_def->sem_depend_2};
	char sem_depends_count = worker_def->sem_depend_2 ? 2 : worker_def->sem_depend_1 ? 1 : 0;

	while (1)
	{
		sleep(worker_def->delay);
		for (char i = 0; i < sem_depends_count; i++)
		{
			if (0 != sem_wait(sem_depends[i]))
			{
				fprintf(stderr, "%s\nFailed to sem_wait\n", ERROR_COLOR);
				perror("");
				exit(EXIT_FAILURE);
			}
		}

		printf("%s[%s] Created detail!%s\n", worker_def->color, worker_def->name, NORMAL_COLOR);

		if (!worker_def->sem_out)
			continue;
		if (0 != sem_post(worker_def->sem_out))
		{
			fprintf(stderr, "%s\nFailed to sem_post\n", ERROR_COLOR);
			perror("");
			exit(EXIT_FAILURE);
		}
	}
}

void sem_try_init(sem_t *sem)
{
	if (0 != sem_init(sem, 0, 0))
	{
		fprintf(stderr, "%s\nFailed to sem_init\n", ERROR_COLOR);
		perror("");
		exit(EXIT_FAILURE);
	}
}

int main()
{
	pthread_t threads[5];
	Worker_Definition worker_defs[5] = {
		{"A", WHITE_COLOR, 1, NULL, NULL, &sem_a},
		{"B", BLUE_COLOR, 2, NULL, NULL, &sem_b},
		{"C", CYAN_COLOR, 3, NULL, NULL, &sem_c},
		{"AB", MAGENTA_COLOR, 0, &sem_a, &sem_b, &sem_ab},
		{"WIDGET", YELLOW_COLOR, 0, &sem_ab, &sem_c, NULL}};

	sem_try_init(&sem_a);
	sem_try_init(&sem_b);
	sem_try_init(&sem_c);
	sem_try_init(&sem_ab);

	for (unsigned char i = 0; i < 5; i++)
		if (pthread_create(&threads[i], NULL, worker_run, &worker_defs[i]))
		{
			fprintf(stderr, "%s\nFailed to pthread_create\n", ERROR_COLOR);
			perror("");
			exit(EXIT_FAILURE);
		}

	while (1)
	{
		sleep(100);
	}
}