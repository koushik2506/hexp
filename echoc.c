#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define ADDR	"dingdong"
#define PORT	"3100"

int main(int argc, char **argv)
{
	int cfd;
	int ret;
	struct addrinfo *result, *rp;
	struct addrinfo hints;
	char buff[32];


	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	ret = getaddrinfo(ADDR, PORT, &hints, &result);

	if ( ret != 0 ) {

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);

	}

	for ( rp = result; rp != NULL; rp = rp->ai_next )
	{
		cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if  ( cfd == -1 ) continue;

		if ( connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1 )
			break;

		perror("connect:");

		close(cfd);
	}

	if ( rp == NULL ) {

		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);

	ret = read(cfd, buff, 32);

	if ( ret != -1 ) {

		printf("%s\n", buff);
		
		exit(EXIT_SUCCESS);
	}

	exit(EXIT_FAILURE);

	return 0;
}



