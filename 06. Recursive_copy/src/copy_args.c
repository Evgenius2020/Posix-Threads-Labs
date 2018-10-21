#include "copy_args.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char *alloc_path_concat(char *root, char *path)
{
    char *result = malloc(strlen(root) + strlen(path) + 2);

    strcpy(result, root);
    strcat(result, "/");
    strcat(result, path);

    return result;
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
