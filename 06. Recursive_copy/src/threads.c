#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "copy_args.h"
#include "stack.h"
#include "threads.h"

void *copy_directory(void *copy_args_raw)
{
    Copy_Args *copy_args = copy_args_raw;
    char *catalog = copy_args->src_path;
    DIR *dir = opendir(catalog);

    struct dirent *dir_entry = NULL;
    // From man:
    int name_max = pathconf(catalog, _PC_NAME_MAX);
    if (name_max == -1) /* Limit not defined, or error */
        name_max = 255;
    size_t len = offsetof(struct dirent, d_name) + name_max + 1;
    struct dirent *dir_entry_prev = malloc(len);

    printf("%s/ [started]\n", catalog);
    // More about MODE: http://man7.org/linux/man-pages/man7/inode.7.html
    mkdir(copy_args->dest_path, S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO);

    Stack *childs = stackCreate();

    for (;;)
    {
        readdir_r(dir, dir_entry_prev, &dir_entry);
        if (!dir_entry)
            break;
        else if (dir_entry->d_type == DT_DIR)
        {
            if (!(strcmp(dir_entry->d_name, ".") * strcmp(dir_entry->d_name, "..")))
                continue;
            if (dir_entry->d_ino == DEST_DIR_INODE)
                continue;

            char *next_src_path = alloc_path_concat(copy_args->src_path, dir_entry->d_name);
            char *next_dest_path = alloc_path_concat(copy_args->dest_path, dir_entry->d_name);
            Copy_Args *next_copy_args = malloc(sizeof(Copy_Args));
            next_copy_args->src_path = next_src_path;
            next_copy_args->dest_path = next_dest_path;

            pthread_t *child = malloc(sizeof(pthread_t));
            while (pthread_create(child, NULL, copy_directory, next_copy_args))
                ;
            stackPush(childs, child);
        }
        else if (dir_entry->d_type == DT_REG)
        {
            char *next_src_path = alloc_path_concat(copy_args->src_path, dir_entry->d_name);
            char *next_dest_path = alloc_path_concat(copy_args->dest_path, dir_entry->d_name);
            Copy_Args *next_copy_args = malloc(sizeof(Copy_Args));
            next_copy_args->src_path = next_src_path;
            next_copy_args->dest_path = next_dest_path;

            pthread_t *child = malloc(sizeof(pthread_t));
            while (pthread_create(child, NULL, copy_file, next_copy_args))
                ;
            stackPush(childs, child);
        }
    }

    while (!stackIsEmpty(childs))
    {
        pthread_t *child = stackPop(childs);
        pthread_join(*child, NULL);
        free(child);
    }

    printf("%s/ [complete]\n", catalog);

    stackDestroy(childs);

    free(dir_entry_prev);
    free(dir_entry);
    closedir(dir);

    free(copy_args->src_path);
    free(copy_args->dest_path);
    free(copy_args);
}

void *copy_file(void *copy_args_raw)
{
    Copy_Args *copy_args = copy_args_raw;
    FILE *src_file, *dest_file;

    printf("%s [started]\n", copy_args->src_path);

    while (NULL == (src_file = fopen(copy_args->src_path, "rb")))
        ;
    while (NULL == (dest_file = fopen(copy_args->dest_path, "wb")))
        ;

    size_t n, m;
    unsigned char buff[8192];
    do
    {
        n = fread(buff, 1, sizeof buff, src_file);
        if (n)
            m = fwrite(buff, 1, n, dest_file);
        else
            m = 0;
    } while ((n > 0) && (n == m));

    printf("%s [complete]\n", copy_args->src_path);

    if (fclose(src_file))
        perror("close input file");
    if (fclose(dest_file))
        perror("close output file");

    free(copy_args->src_path);
    free(copy_args->dest_path);
    free(copy_args);
}