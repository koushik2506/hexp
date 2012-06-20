#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define ADDR	"dingdong"
#define PORT	"3100"
#define LISTEN_BACKLOG	50
#define WEIGHT 10000

void handle_client(int cfd)
{
	int i;
	int prod;
	char prod_str[8];

	memset(prod_str, 0, 8);

	i = 1;
	prod = 1;
	while ( i <= WEIGHT ) {
		
		prod = prod+i;
		i++;
	}

	snprintf(prod_str, 8, "%d\n", prod);

	write(cfd, prod_str, 8);

	close(cfd);
}


int main(int argc, char **argv)
{
	int sfd, cfd;
	int ret;
	struct addrinfo *result, *rp;
	struct addrinfo hints;
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
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
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if ( sfd == -1 ) continue;

		if  ( bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0 ) break;

		perror("bind:");

		close(sfd);

	}

	if ( rp == NULL ) {

		fprintf( stderr, "bind failed\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);

	if ( listen(sfd, LISTEN_BACKLOG) == -1 ){

		perror("listen:");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	peer_addr_len = sizeof(struct sockaddr);

	while ( (cfd = accept(sfd, &peer_addr, &peer_addr_len)) != -1 )
	{
		handle_client(cfd);
		peer_addr_len = sizeof(struct sockaddr);
	}

	perror("accept:");

	close(sfd);

	exit(EXIT_SUCCESS);

	return 0;

}
