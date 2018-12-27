#include "lib/console_app_tools.h"
#include <aio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 8192
#define LINES_LEFT_DEFAULT 25

struct aiocb create_aiorq(int fildes, char *buf, int nbytes)
{
    struct aiocb res;
    memset(&res, 0, sizeof(res));
    res.aio_fildes = fildes;
    res.aio_buf = buf;
    res.aio_nbytes = nbytes;

    return res;
}

void receiving_routine(int socketfd)
{
    unsigned lines_left = LINES_LEFT_DEFAULT;
    char buffer[BUFSIZE];
    int buffer_bytes_count = 0;
    int bytes_to_write = 0;
    char is_socketfd_eof = 0;
    char user_input_key;
    struct aiocb receive_request, write_request, user_input_request;

    while (!is_socketfd_eof || buffer_bytes_count)
    {
        // receive_request, write_request, user_input_request
        struct aiocb *requests[3] = {NULL, NULL, NULL};

        if (lines_left)
        {
            int pos;
            for (pos = 0; pos < buffer_bytes_count; pos++)
                if (buffer[pos] == '\n')
                {
                    lines_left--;
                    if (!lines_left)
                    {
                        pos++;
                        break;
                    }
                }
            bytes_to_write = pos;
        }

        if (!is_socketfd_eof && !buffer_bytes_count)
        {
            receive_request = create_aiorq(socketfd, buffer, BUFSIZE);
            requests[0] = &receive_request;
            aio_read(&receive_request);
        }
        if (bytes_to_write)
        {
            write_request =
                create_aiorq(STDOUT_FILENO, buffer, bytes_to_write);
            requests[1] = &write_request;
            aio_write(&write_request);
        }
        if (!lines_left)
        {
            user_input_request =
                create_aiorq(STDIN_FILENO, &user_input_key, 1);
            requests[2] = &user_input_request;
            aio_read(&user_input_request);
        }

        if (aio_suspend((const struct aiocb *const *)requests, 3, NULL))
            throw_and_exit("aio_suspend");

        if (requests[0] && (aio_error(requests[0]) != EINPROGRESS))
        {
            int res = aio_error(requests[0]);
            if (res)
                throw_with_code_and_exit("aio_read", res);
            int bytes_read = aio_return(requests[0]);
            if (-1 == bytes_read)
                throw_and_exit("read");
            else if (0 == bytes_read)
                is_socketfd_eof = 1;
            else
                buffer_bytes_count = bytes_read;
        }

        if (requests[1] && (aio_error(requests[1]) != EINPROGRESS))
        {
            int res = aio_error(requests[1]);
            if (res)
                throw_with_code_and_exit("aio_write", res);
            int nbytes = requests[1]->aio_nbytes;
            if (-1 == aio_return(requests[1]))
                throw_and_exit("write");

            bytes_to_write = 0;
            buffer_bytes_count -= nbytes;
            memmove(buffer, buffer + nbytes, buffer_bytes_count);

            if (!lines_left)
                printf("%s\nPress enter to scroll down.%s\n", YELLOW_COLOR, WHITE_COLOR);
        }

        if (requests[2] && (aio_error(requests[2]) != EINPROGRESS))
            if (user_input_key == '\n')
                lines_left = LINES_LEFT_DEFAULT;
    }
}