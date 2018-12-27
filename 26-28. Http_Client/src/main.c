#include "lib/console_app_tools.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void receiving_routine(int socketfd);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: <URL>");
		exit(EXIT_FAILURE);
	}
	char *url = argv[1];

	struct addrinfo hints, *gai_res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int gai_error = getaddrinfo(url, "http", &hints, &gai_res);
	if (gai_error)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_error));
		exit(EXIT_FAILURE);
	}

	int socketfd = socket(gai_res->ai_family, gai_res->ai_socktype, gai_res->ai_protocol);
	if (socketfd < 0)
		throw_and_exit("socket");

	if (connect(socketfd, gai_res->ai_addr, gai_res->ai_addrlen))
		throw_and_exit("connect");
	freeaddrinfo(gai_res);

	char request_text[4096];
	char *format_string = "GET / HTTP/1.1\r\nHost: %s\r\n\r\n";
	sprintf(request_text, format_string, url);
	printf("%s%s%s\n", GREEN_COLOR, request_text, WHITE_COLOR);

	printf("%sSending request..\n", YELLOW_COLOR);
	write(socketfd, request_text, strlen(request_text));
	printf("%sRequest has sent.\n", YELLOW_COLOR);

	printf("%sReceiving response..%s\n", YELLOW_COLOR, WHITE_COLOR);
	receiving_routine(socketfd);
	printf("%sResponse has received.\n", GREEN_COLOR);

	close(socketfd);
	exit(EXIT_SUCCESS);
}