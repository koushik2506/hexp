#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define ADDR	"dingdong"
//#define ADDR	"tingtong"
#define PORT	"3100"
#define LISTEN_BACKLOG	50
#define WEIGHT 10000
#define SLEEP_WEIGHT	500 /* 500 ms*/
//#define USE_THREADS
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
	char c = 0;

	memset(read_buf, 0, COMMAND_LEN);
	ret = read(icfd, read_buf, COMMAND_LEN);

	if ( ret != -1 ) {

		if ( strncmp(read_buf, COMMAND, strlen(COMMAND)) == 0 )
		{
			printf("Got %s \n", read_buf);
			usleep(SLEEP_WEIGHT);
			write(icfd, prod_str, 8);
		}
		else {
			printf("read_buf: %s\n", read_buf);
		}
	}
	
//	shutdown(icfd, SHUT_RDWR);
	
//	close(icfd);

	ret = read(icfd, &c, 1);

	printf("ret is %d\n", ret);

	shutdown(icfd, SHUT_RDWR);
	close(icfd);

#ifdef USE_THREADS
	pthread_exit(NULL);
#endif
	

	return;
}

void sig_func(void *arg)
{
	int s, sig;
	sigset_t *set = (sigset_t *)arg;

	while ( 1 ) {

		s = sigwait(set, &sig);

		if ( sig != 0 ) {
			fprintf(stderr, "Got signal: %d\n", sig);
			break;
		}

	}
#ifdef USE_THREADS
	pthread_exit(NULL);
#endif
}

void accept_func(void *sfd_arg)
{
	struct sockaddr peer_addr;
	int cfd, sfd;
	size_t peer_addr_len;

	sfd = *(int *)sfd_arg;

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
#ifdef USE_THREADS
	pthread_exit(NULL);
#endif

}


int main(int argc, char **argv)
{

	int sfd;
	int ret;
	pthread_t sig_thread, accept_thread;
	pthread_attr_t attr;
	sigset_t set;
	struct addrinfo *result, *rp;
	struct addrinfo hints;
	socklen_t peer_addr_len;
	
	sigemptyset(&set);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGUSR1);

	ret = pthread_sigmask(SIG_BLOCK, &set, NULL);
	
	if ( ret ) {

		perror("pthread_sigmask:");
		exit(EXIT_FAILURE);
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	ret = pthread_create(&sig_thread, &attr, (void *)sig_func, (void *)&set);

	if ( ret ) {

		perror("pthread_create:");
		exit(EXIT_FAILURE);
	}
	
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
		int sock_opt = 1;
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if ( sfd == -1 ) continue;

		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));

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

	ret = pthread_create(&accept_thread, NULL, (void *)accept_func, (void *)&sfd);

	if ( ret ) {

		perror("accept thread pthread_create:");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	ret = pthread_join(sig_thread, NULL);

	if ( ret ) perror("pthread_join:");

	printf("Closing\n");

	close(sfd);

	exit(EXIT_SUCCESS);

	return 0;

}
