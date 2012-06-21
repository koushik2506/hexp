#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define ADDR	"dingdong"
#define PORT	"3100"
#define LISTEN_BACKLOG	50
#define WEIGHT 10000
#define SLEEP_WEIGHT	500 /* 500 ms*/
#define USE_THREADS
#define COMMAND	"START"
#define COMMAND_LEN 8

void handle_client_w(int cfd)
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

void handle_client_s(void *cfd)
{
	char prod_str[8] = "1234567";
	char read_buf[COMMAND_LEN];
	int icfd = *(int *)cfd;
	int ret;

	memset(read_buf, 0, COMMAND_LEN);
	ret = read(icfd, read_buf, COMMAND_LEN);

	if ( ret != -1 ) {

		if ( strcmp(read_buf, COMMAND) == 0 )
		{
			usleep(SLEEP_WEIGHT);
			write(icfd, prod_str, 8);
		}
	}

	close(icfd);

	return;
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
		pthread_t t;
		int local_cfd = cfd;
#ifdef USE_THREADS
		pthread_create(&t, NULL,  (void *)handle_client_s, (void *)(&cfd));
		pthread_yield();
#else
		handle_client_s(&local_cfd);
#endif
		peer_addr_len = sizeof(struct sockaddr);
	}

	perror("accept:");

	close(sfd);

	exit(EXIT_SUCCESS);

	return 0;

}
