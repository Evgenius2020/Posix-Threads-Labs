#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>

typedef struct
{
    char *src_path;
    char *dest_path;
} Copy_Args;

char *alloc_path_concat(char *root, char *path)
{
    char *result = malloc(strlen(root) + strlen(path) + 2);

    strcpy(result, root);
    strcat(result, "/");
    strcat(result, path);

    return result;
}

int DEST_DIR_INODE;

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

    printf("%s/ \n", catalog);
    // More about MODE: http://man7.org/linux/man-pages/man7/inode.7.html
    mkdir(copy_args->dest_path, S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO);

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
            copy_directory(next_copy_args);
        }
        else if (dir_entry->d_type == DT_REG)
            printf("%s\n", dir_entry->d_name);
    }

    free(dir_entry_prev);
    free(dir_entry);
    closedir(dir);

    free(copy_args->src_path);
    free(copy_args->dest_path);
    free(copy_args);
}

int set_dest_dir_inode(char *path)
{
    mkdir(path, S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO);
    int fd = open(path, O_RDONLY);

    if (-1 == fd)
        return -1;

    struct stat file_stat;
    if (0 != fstat(fd, &file_stat))
        return -1;

    DEST_DIR_INODE = file_stat.st_ino;

    if (0 != close(fd))
        return -1;

    return 0;
}

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
    copy_args->dest_path = malloc(strlen(argv[2])+ 1);
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