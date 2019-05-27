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

#define BUFFER_SIZE 1024           //Max size of one part of a file
#define FILE_NAME_MAX_SIZE 512     //Max size of file name (including address)   
static char userName[50];          //user name used to identify a client in a chatroom
static int fileReading = 0;        //a flag used to identify whether this client is receiving file

//send file in parts according to file's name and server's socket
void Sendfile(char* Filename, void* Socket)
{
	int *SocketCopy = Socket; 
	char buffer[1025];
	FILE *fp;
	fp = fopen(Filename, "r");

	//cannot open file
	if(NULL == fp)
	{
		printf("File:%s Not Found\n", Filename);
		return;
	}
	else
	{
		//reading the file and send it by parts using while loop
		int length =0;
		while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			write(*SocketCopy, &length, sizeof(int));
			if(write(*SocketCopy, buffer, length) < 0)
			{
				printf("Upload file:%s Failed.\n", Filename);
				break;
			}
			bzero(buffer, BUFFER_SIZE);
		}
	}
	fclose(fp);
	printf("File:%s Upload Successfully!\n", Filename);
	
}

void ReceiveFile(char* dest, int Socket);
//send the message to the server any time terminal get input
void* Send(void* Socket)
{
	char sender[80];
	char Filename[FILE_NAME_MAX_SIZE];
	//save the socked into a int pointer
	int *SocketCopy = Socket;
	while(1)
	{
		//if receiving file, don't get input from terminal
		if(fileReading) continue;
		//else normally working
		fgets(sender, sizeof(sender), stdin);
		
		//if this is the signal to save file, pass the destination to save
		if(sender[1] == 'f' && sender[2] == 's')
		{
			fileReading = 1;
			char destination[50];
			for(int i = 4; i <  strlen(sender) - 1; ++i)
				destination[i - 4] = sender[i];
			destination[strlen(sender) - 5] = '\0';
			//set flag fileReading and start to receive file
			ReceiveFile(destination, *SocketCopy);
			continue;
		}		

		//otherwise, this is a chatting message, distribute it immediately
		int messageSize = strlen(sender) + 1;
		write(*SocketCopy, &messageSize, sizeof(int));
 		write(*SocketCopy, sender, messageSize);      
	
		//check whether this is a quit message, if is, quit immediately
		if(strcmp(sender, ":q!\n") == 0)
			exit(1);
		
		//check whether this is a signal to send file
		else if(sender[1] == 'f' && sender[2] == 'w')
		{	
			printf("please enter the file name again( including address):\n");
			scanf("%s", Filename);
			
			//open the file and send the size of the file to the server
			FILE *fp=fopen(Filename, "r");
			fseek(fp, 0L, SEEK_END);
			int Filesize=ftell(fp); 
			int intSize = sizeof(int);
			write(*SocketCopy, &intSize, sizeof(int));
			write(*SocketCopy, &Filesize, sizeof(int));

			//open this function to send this file, and restore flag after it
	                Sendfile( Filename, SocketCopy );
			fileReading = 0;
		}		
	}
}


//receive file from server, save it in destination
void ReceiveFile(char* dest, int Socket)
{
	//be prepared to receive file
	char buffer[BUFFER_SIZE];
	printf("the position you want to save file in is %s\n", dest);
	FILE *fp = fopen(dest, "w");
	if(NULL == fp)
	{
		printf("File:\t%s Can Not Open To Write\n", dest);
		exit(1);
	}
	bzero(buffer, BUFFER_SIZE);

	//read the size of the file, turn string into int
	char filesize[20];
	char filesizeStringSize[2];
	int L1 = read(Socket, filesizeStringSize, 2);
	int L2 = read(Socket, filesize, atoi(filesizeStringSize) + 1);
	int filesizeInt = atoi(filesize);

	//prepare to receive the file
	int length = 0;
	int i = 0;
	fileReading = 1;

	//receiving the file in parts according to file size
	while(i < filesizeInt/1024 + 1)
	{	
		length = read(Socket, buffer, BUFFER_SIZE); 
		if(fwrite(buffer, sizeof(char), length - 2, fp) < length - 2)
		{
			printf("File:\t%s Write Failed\n", dest);
			return;
		}
		printf("file receiving part %d successfully!\n", ++i);
		bzero(buffer, BUFFER_SIZE);
	}

	//print success message and free neccessary things
	printf("Receive File From Server Successful into %s!\n", dest);
	fileReading = 0;
	fclose(fp);
}

//receive message from server
void* Receive(void* Socked)
{
	int *SockedCopy = Socked;
	char Receiver[80];

	while(1){
		if(fileReading == 1)
			continue;
		//read message continuosly
		int reveiverEnd = 0;
		reveiverEnd  = read (*SockedCopy, Receiver, 1000);
		if(Receiver[0] == '!' && Receiver[1] == '!')
			fileReading = 1;
		Receiver[reveiverEnd] = '\0';	
		fputs(Receiver, stdout);
		Receiver[0] = '\0';
	}
}
int main ()
{
	int sockfd, n, MessageSize;
	pthread_t threadSend;
	pthread_t threadReceive;
	struct sockaddr_in serv, cli;
	char rec[1000];
	char send[80];
	char serAddress[80];
	
	//input server address
	//use defualt when typing in changeline
	printf("Input server address(type <Enter> to use default): ");
	fgets(serAddress, sizeof(serAddress), stdin);
	if(serAddress[0] == '\n')
	{
		strcpy(serAddress, "127.0.0.1\n");
	}
	serAddress[strlen(serAddress) - 1] = '\0';

	//input UserName
	Start: printf("Input Username: " );
	fgets(userName, sizeof(userName), stdin);
	userName[strlen(userName) - 1] = '\0'; //cut the '\n' ending
 	MessageSize = strlen(userName);

	//create socket
 	sockfd = socket (PF_INET, SOCK_STREAM, 0);

	//create server information
 	bzero (&serv, sizeof (serv));
	serv.sin_family = PF_INET;
	serv.sin_port = htons (8888);
 	serv.sin_addr.s_addr = inet_addr (serAddress /*server address*/);

	//connect to the server
	if(connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr)) == -1)
	{
		printf("connect %s failed\n", serAddress);
		exit(1);
	}

	//send the user name to the server
	write(sockfd, &MessageSize, sizeof(int));
 	write (sockfd, userName, sizeof(userName));

	//get successfully connect message
	n = read (sockfd, rec, 1000);
 	rec[n] = '\0';	

	//check whether been rejected according to rec
	if(rec[0] == 'R')
	{
		//Reject GOTO input another userName
		rec[0] = '\0';
		printf("Username existed, choose another one.\n");
	 	goto Start; 
	}
	else
	{
		//doesn't been rejected, open threads that are needed
		fputs(rec, stdout);
		//open send thread 	
		pthread_create(&threadSend, 0, Send, &sockfd);
		//open receiving message thread
		pthread_create(&threadReceive, 0, Receive, &sockfd);
	}

	//Make sure this program last a long time
	for(int i = 0; i < 100; ++i)
		sleep(100000);
	
	//close and free everything then quit
	pthread_exit(&threadSend);
	pthread_exit(&threadReceive);
	close(sockfd);
	
	return 0;
}
