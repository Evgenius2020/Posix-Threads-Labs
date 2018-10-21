#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "copy_args.h"
#include "threads.h"

int main(int argc, char *argv[])
{
    pthread_t thread;

    if (argc < 3)
    {
        fprintf(stderr,
                "Usage: %s 'src_path' 'dest_path' \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Copy_Args *copy_args = malloc(sizeof(Copy_Args));
    copy_args->src_path = malloc(strlen(argv[1]) + 1);
    strcpy(copy_args->src_path, argv[1]);
    copy_args->dest_path = malloc(strlen(argv[2]) + 1);
    strcpy(copy_args->dest_path, argv[2]);

    if (0 != set_dest_dir_inode(copy_args->dest_path))
    {
        perror("Process: Failed set 'DEST_DIR_INODE'");
        exit(EXIT_FAILURE);
    }

    if (0 != pthread_create(&thread, NULL, copy_directory, copy_args))
    {
        perror("Process: Failed to create thread");
        exit(EXIT_FAILURE);
    }
    printf("Process: created a thread!\n");

    if (0 != pthread_join(thread, NULL))
    {
        perror("Process: Failed to join thread");
        exit(EXIT_FAILURE);
    }
    printf("Process: Joined a thread!\n");

    exit(EXIT_SUCCESS);
}