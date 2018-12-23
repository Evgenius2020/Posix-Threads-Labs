#include "console_app_tools.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char *url;
#define BUFSIZE 8192
#define LINES_MAX 25

void send_request(int socketfd)
{
	char request_text[4096];
	char *format_string = "GET / HTTP/1.1\nHost: %.*s\n\n";
	sprintf(request_text, format_string, sizeof(request_text) - strlen(format_string), url);
	printf("%s%s%s\n", GREEN_COLOR, request_text, WHITE_COLOR);
	write(socketfd, request_text, strlen(request_text));
}

void start_client_routine(int socketfd)
{
	fd_set readfds, writefds;
	int ready, buffer_bytes_count = 0;

	unsigned lines_count = 0;
	char *buffer = malloc(sizeof(char) * BUFSIZE);
	int buffer_read_position = 0;
	char is_socketfd_eof = 0;
	char is_nothing_to_print = 1;
	char is_buffer_full = 0;
	unsigned blocks_count = 1;

	printf("%sSending request..\n", YELLOW_COLOR);
	send_request(socketfd);
	printf("%sReques has sent.\n", YELLOW_COLOR);	
	printf("%sReceiving response..%s\n", YELLOW_COLOR, WHITE_COLOR);	

	while (!is_socketfd_eof || !is_nothing_to_print)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		if (!is_socketfd_eof)
			FD_SET(socketfd, &readfds);
		if (!is_nothing_to_print && lines_count < LINES_MAX)
			FD_SET(STDOUT_FILENO, &writefds);
		if (lines_count >= LINES_MAX)
			FD_SET(STDIN_FILENO, &readfds);

		ready = select(socketfd + 1, &readfds, &writefds, NULL, NULL);

		if (-1 == ready)
			throw_and_exit("select");
		else if (!ready)
			continue;

		if (FD_ISSET(socketfd, &readfds))
		{
			int offset = 0;
			int len = BUFSIZE;
			if (!is_nothing_to_print && is_buffer_full)
			{
				blocks_count++;
				buffer = realloc(buffer, BUFSIZE * blocks_count);
				offset = BUFSIZE * (blocks_count - 1);
			}
			else if (!is_nothing_to_print && !is_buffer_full)
			{
				offset = BUFSIZE * (blocks_count - 1) + (buffer_bytes_count % BUFSIZE);
				len = BUFSIZE * blocks_count - offset;
			}
			int bytes_read = read(socketfd, buffer + offset, len);

			is_nothing_to_print = 0;
			buffer_bytes_count += bytes_read;

			if (-1 == bytes_read)
				throw_and_exit("read");
			else if (bytes_read == 0)
				is_socketfd_eof = 1;
			else if (buffer_bytes_count / blocks_count == BUFSIZE)
				is_buffer_full = 1;
			else
				is_buffer_full = 0;
		}

		if (FD_ISSET(STDOUT_FILENO, &writefds))
		{
			int offset = buffer_read_position;
			int i;
			for (i = buffer_read_position; i < BUFSIZE * blocks_count; i++)
				if (buffer[i] == '\n')
				{
					lines_count++;
					if (lines_count == LINES_MAX)
						break;
				}
			buffer_read_position = i + 1;

			int write_res = write(STDOUT_FILENO, buffer + offset, buffer_read_position - offset);
			if (-1 == write_res)
				throw_and_exit("write");
			else if (buffer_read_position >= buffer_bytes_count)
				is_nothing_to_print = 1;
			else if (lines_count == LINES_MAX)
				printf("%sPress enter to scroll down.%s\n", YELLOW_COLOR, WHITE_COLOR);
		}

		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			char key;
			read(STDIN_FILENO, &key, 1);
			if (key == '\n')
				lines_count = 0;
		}
	}
	printf("%sResponse has received.%s\n", YELLOW_COLOR, WHITE_COLOR);	

	free(buffer);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	if (argc != 2)
	{
		fprintf(stderr, "Usage: <URL>");
		exit(EXIT_FAILURE);
	}
	url = argv[1];

	struct addrinfo hints, *gai_res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	ret = getaddrinfo(url, "http", &hints, &gai_res);
	if (ret)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	int socketfd = socket(gai_res->ai_family, gai_res->ai_socktype, gai_res->ai_protocol);
	if (socketfd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (connect(socketfd, gai_res->ai_addr, gai_res->ai_addrlen))
		throw_and_exit("connect");
	freeaddrinfo(gai_res);

	start_client_routine(socketfd);

	exit(EXIT_SUCCESS);
}