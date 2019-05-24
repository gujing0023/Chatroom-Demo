#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

static pthread_t thread;
static pthread_t threadClient[100];
static int ServerSock;
static int clientNumber;


//Every client's information
typedef struct
{
	pthread_t threadNumber;
	int sock;
	char UserName[16]; 
	struct sockaddr address;
	int addr_len;
} connection_t;
static connection_t conn[100];



//this function distributes the messsage/status of single client to the other
//Info is the message needed to be distributed

int SendInfo(void* Info)
{
	char *info = Info;
	for(int i = 0; i < 100; ++i)
		//send to the client that exists and doesn't quit room
		if(conn[i].addr_len != -1 && conn[i].addr_len != 0){
			if(send (conn[i].sock, info , strlen(info) + 1, 0) == -1)
				printf("error occured, send to %s fail", conn[i].UserName);
			printf("--��D�_ -send <%s> to <%s> successfully!\n", info, conn[i].UserName);
		}	
	return 0;	
}



//this function sends the file of single client to the others
//File is the filename needed to be sent
int SendFile(char* Filename, void* clientStruct)
{
	char*filename = Filename;
	int size;
	int filesize;
	char buffer[1024];
	int len;
	connection_t* clientInfo = (connection_t *)clientStruct;	
	read(clientInfo->sock, &size, sizeof(int));
        read(clientInfo->sock, &filesize, sizeof(int));
	for(int i=0; i < filesize/1024+1; ++i)
	{
		read(clientInfo->sock, &len, sizeof(int));
		read(clientInfo->sock, buffer, len);
		SendInfo(buffer);
	}	
	return 0;
}
//This function deals with single client, aim to receive message from this client
//and then send them to another using SendInfo
void* Receive(void* clientStruct)
{

	connection_t* clientInfo = (connection_t *)clientStruct;
	while(1)
	{
		//read the message from the client
		char *Buffer;
		int messageLen = 0;
		read(clientInfo->sock, &mes��D�_ 				char sign[] = "!!";��D�_ 
                                char file[] = " send you a file named as ";
				char fileMessage[50];
				char Filename[FILE_NAME_MAX_SIZE];
				fileMessage[0] = '\0';
				strcat(fileMessage, clientInfo->UserName);
				strcat(fileMessage, file);
				//read the file name  from buffer
				//send the file to the others
				for(int t = 3; t < messageLen-2; t++)
					Filename[t-3] = Buffer[t];
				Filename[messageLen-5]='\0';
				strcat(fileMessage, Filename);
				strcat(sign, fileMessage);
				SendInfo(sign);
				printf("%s\n", Filename);
				SendFile(Filename, clientInfo);
			}
			else{
				//constitute the message
				char begin[] = " says";
				char messageDistribute[200];
				messageDistribute[0] = '\0';
				strcat(messageDistribute, clientInfo->UserName);
				strcat(messageDistribute, begin);
				strcat(messageDistribute, Buffer);
				SendInfo(messageDistribute);
			}
			free(Buffer);
		}
		else
			continue;
	}
}

//determine whether userName already exist
//return 1 if ex��D�_ isted, otherwise 0
int usernameExisted(char userName[], int clientnumber)
{
	for(int i = 0; i < 100 && i != clientnumber; ++i)
	{
		if(conn[i].addr_len != 0 && conn[i].addr_len != -1)
			if(strcmp(conn[i].UserName, userName) == 0)
				return 1;

	}	
	return 0;
}


//aim to accept whenever there is a client trying to connect
void * process(void * ptr)
{
	pthread_t clientThread[100];
	char * buffer;
	int len;
	//the number of the client connecting now
	clientNumber = 0;      
	long addr = 0;
	while(1){
		//waiting to be connected
		if(clientNumber < 100)
		{
			conn[clientNumber].sock = accept(ServerSock, &conn[clientNumber].address, &conn[clientNumber].addr_len);
		}
		else
			break;


		//the length of the message
		read(conn[clientNumber].sock, &len, sizeof(int));
		if (len > 0)
		{

			//multiple information of a client
			addr = (long)((struct sockaddr_in *)&conn[clientNumber].address)->sin_addr.s_addr;
			buffer = 