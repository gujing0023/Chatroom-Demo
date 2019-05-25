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

#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
static char userName[50];
static int senderMyself;
static int fileReading = 0;

void* Sendfile(char* Filename, void* Socked)
{
	int *SockedCopy = Socked; 
	char *filename = Filename;
	char buffer[1025];
	FILE *fp;
	fp = fopen(filename, "r");
	if(NULL == fp)
	{
		printf("File:%s Not Found\n", filename);
	}
	else
	{
		buffer[0]='\0';
		int length =0;
		while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			write(*SockedCopy, &length, sizeof(int));
			if(write(*SockedCopy, buffer, length) < 0)
			{
				printf("Upload file:%s Failed.\n", filename);
				break;
			}
			bzero(buffer, BUFFER_SIZE);
		}
	}
	fclose(fp);
	printf("File:%s Upload Successfully!\n", filename);
	
}

void ReceiveFile(char* dest, int Socket);
//send the message to the server any time terminal get input
void* Send(void* Socked)
{
	char sender[80];
	char Filename[FILE_NAME_MAX_SIZE];
	//save the socked into a int pointer
	int *SockedCopy = Socked;
	while(1)
	{
		if(fileReading) continue;
		fgets(sender, sizeof(sender), stdin);
		//if this is the signal to save file, pass the destination to save
		if(sender[1] == 'f' && sender[2] == 's')
		{
			fileReading = 1;
			char destination[50];
			for(int i = 4; i <  strlen(sender) - 1; ++i)
				destination[i - 4] = sender[i];
			destination[strlen(sender) - 5] = '\0';
			ReceiveFile(destination, *SockedCopy);
			continue;
		}		
		//else this is the message needed to be sent to the sever
		int messageSize = strlen(sender) + 1;
		write(*SockedCopy, &messageSize, sizeof(int));
 		write(*SockedCopy, sender, messageSize);      
		//check whether this is a quit message
		if(strcmp(sender, ":q!\n") == 0)
			exit(1);
		else if(sender[1] == 'f' && sender[2] == 'w')
		{	
			printf("please enter the file name again( including address):\n");
			scanf("%s", Filename);
			FILE *fp=fopen(Filename, "r");
			fseek(fp, 0L, SEEK_END);
			int Filesize=ftell(fp); 
			int intSize = sizeof(int);
			write(*SockedCopy, &intSize, sizeof(int));
			write(*SockedCopy, &Filesize, sizeof(int));
	                Sendfile( Filename, SockedCopy );
			fileReading = 0;
		}		
	}
}


//receive file from server
void ReceiveFile(char* dest, int Socket)
{
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
	//start receiving the file
	int length = 0;
	int i = 0;
	fileReading = 1;
	while(i < filesizeInt/1024 + 1)
	{
		length = read(Socket, buffer, BUFFER_SIZE); 
		if(fwrite(buffer, sizeof(char), length - 2, fp) < length - 2)
		{
			printf("File:\t%s Write Failed\n", dest);
			break;
		}
		printf("file receiving part %d successfully!\n", ++i);
		bzero(buffer, BUFFER_SIZE);
	}
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
	int sockfd, n;
	pthread_t threadSend;
	pthread_t threadReceive;
	struct sockaddr_in serv, cli;
	char rec[1000];
	char send[80];
	char serAddress[80];
	
	//input server address
	printf("Input server address(type <Enter> to use default): ");
	fgets(serAddress, sizeof(serAddress), stdin);
	if(serAddress[0] == '\n')
	{
		strcpy(serAddress, "127.0.0.1\n");
	}
	serAddress[strlen(serAddress) - 1] = '\0';

	//input UserName
	userName[0] = '\0';
	Start: printf("Input Username: " );
	fgets(userName, sizeof(userName), stdin);
	userName[strlen(userName) - 1] = '\0';
 	int MessageSize = strlen(userName);

	//create socked
 	sockfd = socket (PF_INET, SOCK_STREAM, 0);

	//create server information
 	bzero (&serv, sizeof (serv));
	serv.sin_family = PF_INET;
	serv.sin_port = htons (8888);
 	serv.sin_addr.s_addr = inet_addr (serAddress /*server address*/);

	//connect to the server
	if(connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr)) == -1)
	{
		printf("connect failed\n");
		exit(1);
	}

	//send the user name to the server
	write(sockfd, &MessageSize, sizeof(int));
 	write (sockfd, userName, sizeof(userName));

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
