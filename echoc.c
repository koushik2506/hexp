#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define ADDR	"dingdong"
//#define ADDR	"tingtong"
#define PORT	"3100"
#define NUM_THREADS	5
#define COMMAND	"START"
#define COMMAND_LEN 6
#define MULTIPLE_CLIENTS

void print_time_diff(char *s, struct timeval t1, struct timeval t2)
{
	if ( t1.tv_sec == t2.tv_sec )
		fprintf(stderr, "%s: %ld\n", s, t2.tv_usec - t1.tv_usec);
	else
		fprintf(stderr, "%s: %ld\n", s, ((t2.tv_sec - t2.tv_sec)*1000*1000) + (1000*1000 - t1.tv_usec) + t2.tv_usec);
}

int client_work()
{
	int cfd;
	int ret;
	struct addrinfo *result, *rp;
	struct addrinfo hints;
	char buff[32], c;
	struct timeval t_initial, t_final;


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
		return -1;

	}

	for ( rp = result; rp != NULL; rp = rp->ai_next )
	{
		cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if  ( cfd == -1 ) continue;

		gettimeofday(&t_initial, NULL);
		if ( connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1 ) {
			gettimeofday(&t_final, NULL);
			print_time_diff("CONNECT", t_initial, t_final);
			break;
		}

		perror("connect:");

		close(cfd);
	}

	if ( rp == NULL ) {

		fprintf(stderr, "Could not connect\n");
		return -1;
	}

	freeaddrinfo(result);

	gettimeofday(&t_initial, NULL);
	ret = write(cfd, COMMAND, COMMAND_LEN);
	gettimeofday(&t_final, NULL);
	print_time_diff("WRITE", t_initial, t_final);

	gettimeofday(&t_initial, NULL);
	ret = read(cfd, buff, 32);
	gettimeofday(&t_final, NULL);

	shutdown(cfd, SHUT_RDWR);

	close(cfd);

	print_time_diff("READ", t_initial, t_final);

	if ( ret != -1 ) {

		printf("%s\n", buff);

		return 0;
		
	}

	return -1;
}


int main(int argc, char **argv)
{
#ifdef MULTIPLE_CLIENTS
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;

	int rc;
	long t;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for ( t=0; t<NUM_THREADS; t++) {

		rc = pthread_create(&threads[t], &attr, (void *)client_work, NULL);

		if ( rc ) perror("pthread_create:");

	}

	/* Wait till the jobs are done */

	for ( t=0; t<NUM_THREADS; t++ ) {

		rc = pthread_join(threads[t], NULL);

		if ( rc ) perror("pthread_join:");
	}

	return 0;
#else
	client_work();
	return 0;
#endif
	
}
