/* tcp-client.c */
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

//send the message to the server any time terminal get input
void*  Send(void* Socked)
{
	char sender[80];
	//save the socked into a int pointer
	int *SockedCopy = Socked;
	while(fgets(sender, sizeof(sender), stdin)){
		printf("\r");
		//whenever enter a string, send it
		int messageSize = strlen(sender) + 1;
		write(*SockedCopy, &messageSize, sizeof(int));
 		int i = write(*SockedCopy, sender, messageSize);      //both work when sending message
		//check whether this is a quit message
		if(strcmp(sender, ":q!\n") == 0)
			exit(1);
		
	}
}


//receive message from server
void* Receive(void* Socked)
{
	int *SockedCopy = Socked;
	char Receiver[80];

	while(1){
		//read message continuosly
		int reveiverEnd = 0;
		reveiverEnd  = read (*SockedCopy, Receiver, 1000);
		Receiver[reveiverEnd] = '\0';	
		fputs(Receiver, stdout);
		Receiver[0] = '\0';
	}
}

int main ()
{
	int sockfd, n;
	pthread_t threadSend;
	pthread_t threadReceive;
	struct sockaddr_in serv, cli;
	char rec[1000];
	char send[80];

	//input UserName
	Start: printf("Input Username:" );
	fgets(send, sizeof(send), stdin);
	send[strlen(send) - 1] = '\0';
 	int MessageSize = strlen(send);

	//create socked
 	sockfd = socket (PF_INET, SOCK_STREAM, 0);

	//create server information
 	bzero (&serv, sizeof (serv));
	serv.sin_family = PF_INET;
	serv.sin_port = htons (8888);
 	serv.sin_addr.s_addr = inet_addr ("127.0.0.1" /*local machine*/);

	//connect to the server
	connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr));
	
	//send the user name to the server
	write(sockfd, &MessageSize, sizeof(int));
 	write (sockfd, send, sizeof(send));
	send[0] = '\0';

	//get successfully connecting message
	n = read (sockfd, rec, 1000);//n marks real length
 	rec[n] = '\0';	

	//check whether been rejected
	if(rec[0] == 'R')
	{
		rec[0] = '\0';
		printf("Username existed, choose another one.\n");
	 	goto Start; 
	}
	else
	{	
		fputs(rec, stdout);

		//open send thread 	
		pthread_create(&threadSend, 0, Send, &sockfd);

		//open receiving message thread
		pthread_create(&threadReceive, 0, Receive, &sockfd);
	}

	//close socked and close two threads
	for(int i = 0; i < 100; ++i)
		sleep(100000);
	pthread_exit(&threadSend);
	pthread_exit(&threadReceive);
	close(sockfd);
	
	return 0;
}
