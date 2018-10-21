#ifndef COPY_ARGS
#define COPY_ARGS

int DEST_DIR_INODE;

typedef struct
{
    char *src_path;
    char *dest_path;
} Copy_Args;

char *alloc_path_concat(char *root, char *path);
int set_dest_dir_inode(char *path);

#endif