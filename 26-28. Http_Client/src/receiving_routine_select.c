#include "lib/console_app_tools.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define BUFSIZE 8192
#define LINES_LEFT_DEFAULT 25

void receiving_routine(int socketfd)
{
    unsigned lines_left = LINES_LEFT_DEFAULT;
    char buffer[BUFSIZE];
    int buffer_bytes_count = 0;
    char is_socketfd_eof = 0;

    fd_set readfds, writefds;
    while (!is_socketfd_eof || buffer_bytes_count)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        if (!is_socketfd_eof && !buffer_bytes_count)
            FD_SET(socketfd, &readfds);
        if (buffer_bytes_count && lines_left)
            FD_SET(STDOUT_FILENO, &writefds);
        if (!lines_left)
            FD_SET(STDIN_FILENO, &readfds);

        int ready = select(socketfd + 1, &readfds, &writefds, NULL, NULL);
        if (-1 == ready)
            throw_and_exit("select");

        if (FD_ISSET(socketfd, &readfds))
        {
            int bytes_read = read(socketfd, buffer, sizeof(buffer));
            if (-1 == bytes_read)
                throw_and_exit("read");
            else if (bytes_read == 0)
                is_socketfd_eof = 1;
            else
                buffer_bytes_count = bytes_read;
        }

        if (FD_ISSET(STDOUT_FILENO, &writefds))
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

            if (write(STDOUT_FILENO, buffer, pos) < pos)
                throw_and_exit("write");
            else
                buffer_bytes_count -= pos;
            if (buffer_bytes_count)
                memmove(buffer, buffer + pos, buffer_bytes_count);

            if (!lines_left)
                printf("%s\nPress enter to scroll down.%s\n", YELLOW_COLOR, WHITE_COLOR);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            char key;
            read(STDIN_FILENO, &key, 1);
            if (key == '\n')
                lines_left = LINES_LEFT_DEFAULT;
        }
    }
}