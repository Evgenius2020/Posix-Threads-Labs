#ifndef THREADS
#define THREADS

void *copy_file(void *copy_args_raw);
void *copy_directory(void *copy_args_raw);

#endif