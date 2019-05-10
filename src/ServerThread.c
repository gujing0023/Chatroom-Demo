#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_SIZE 50
#define NUM_CLIENT 5
void *connection_handler(void *socket_desc);
int main()
{
	int socket_desc , new_socket , c , *new_sock, i;
	pthread_t sniffer_thread;
	for (i=1; i<=NUM_CLIENT; i++) {
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) i) < 0){
			perror("could not create thread");
			return 1;
		}
		sleep(3);
	}
	pthread_exit(NULL);
	return 0;
}

void *connection_handler(void *threadid)
{
	int threadnum = (int)threadid;
	int sock_desc;
	struct sockaddr_in serv_addr;
	char sbuff[MAX_SIZE],rbuff[MAX_SIZE];
	if((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Failed creating socket\n");
		bzero((char *) &serv_addr, sizeof (serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		serv_addr.sin_port = htons(8888);
		if (connect(sock_desc, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
			printf("Failed to connect to server\n");
		}
		printf("Connected successfully client:%d\n", threadnum);
		while(1){
			printf("For thread : %d\n", threadnum);
			fgets(sbuff, MAX_SIZE , stdin);
			send(sock_desc,sbuff,strlen(sbuff),0);
			if(recv(sock_desc,rbuff,MAX_SIZE,0)==0)
				printf("Error");
			else
				fputs(rbuff,stdout);
			bzero(rbuff,MAX_SIZE);
			sleep(2);
		}
		close(sock_desc);
		return 0;
}
