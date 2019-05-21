#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <unistd.h>

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
			printf("---send <%s> to <%s> successfully!\n", info, conn[i].UserName);
		}	
	return 0;	
}


//This function deals with single client, aim to receive message from this client
//and then send them to another using SendIinfo
void* Receive(void* clientStruct)
{
	connection_t* clientInfo = (connection_t *)clientStruct;
	while(1)
	{
		//read the message from the client
		char *Buffer;
		int messageLen = 0;
		read(clientInfo->sock, &messageLen, sizeof(int));
		if(messageLen > 0)
		{
			Buffer = (char *)malloc((messageLen+1)*sizeof(char));
			read(clientInfo->sock, Buffer, messageLen);   // the program stucks here and don't know why
						
			if(Buffer[0] != ':') continue;
			Buffer[messageLen] = '\0';
			//whether the client want to quit
			if( Buffer[1] == 'q' && Buffer[2] == '!' )
			{
				//constitute quit message and delete this client
				char quit[] = " quit the chat room\n";
				char quitMessage[50];		
				char quitNumber[50];
				quitMessage[0] = '\0';
				sprintf(quitNumber, "There are %d people in the Chatroom now!!\n", --clientNumber);
				strcat(quitMessage, clientInfo->UserName);
				strcat(quitMessage, quit);	
				strcat(quitMessage, quitNumber);
				//send the info to the others
				SendInfo(quitMessage);
				clientInfo->addr_len = -1;
				pthread_exit(&clientInfo->threadNumber);
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
//return 1 if existed, otherwise 0
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
			buffer = (char *)malloc((len+1)*sizeof(char));
			buffer[len] = 0;
			read(conn[clientNumber].sock, buffer, len);
			//save client's nick name
			strcpy(conn[clientNumber].UserName, buffer);
		
			//determine whether the UserName existed
			if(usernameExisted(conn[clientNumber].UserName, clientNumber))
			//reject connection
			{
				send(conn[clientNumber].sock,  "Reject", 6, 0);
				--clientNumber;
			}
			else
			{
				//send success message to the client
				send (conn[clientNumber].sock, "You have entered the chatroom, Start CHATTING Now!\n", 51, 0);
				
				//send inform message to all the users
				char mesStart[50] = "User ";
				char mesMiddle[30] = " has entered the Chatroom!\n";
				char mesNumber[50];
				sprintf(mesNumber, "There are %d people in the Chatroom now!!\n", clientNumber + 1);
				strcat(mesStart, conn[clientNumber].UserName);
				strcat(mesStart, mesMiddle);
				strcat(mesStart, mesNumber);
				printf("%s", mesStart);
				SendInfo(mesStart);
				
				//create a thread dealing the messages from a single client
				pthread_create(&threadClient[clientNumber], 0, Receive, &conn[clientNumber]);
				conn[clientNumber].threadNumber = threadClient[clientNumber];
			}
			free(buffer);
		}
		clientNumber += 1;
		
	}
	pthread_exit(0);
}

int main(int argc, char ** argv){
	struct sockaddr_in address;
	int port = 8888;
	connection_t * connection;

	//create socked
	ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//bind the socked to a port
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(ServerSock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "error: cannot bind socket to port %d\n", port);
		return -4;
	}


	//listen for connections
	listen(ServerSock, 100);
	printf("the server is ready and listening\n");


	//creating a thread dealing with connections
	pthread_create(&thread, 0, process, (void *)connection);


	//keep this program working
	for(int i = 0; i < 100; ++i)
		sleep(10000);


	//close socked and thread
	pthread_detach(thread);
	close(ServerSock);
	return 0;
}
