#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <unistd.h>

#define BUFFER_SIZE 1024             //Max size of one single part of a file
#define FILE_NAME_MAX_SIZE 512       //Max size of a file name(including address)

static pthread_t thread;             //mark main thread, process
static pthread_t threadClient[100];  //mark clients' thread
static int ServerSock;               //socket of server
static int clientNumber;             //use to count thr number of clients
static int fileDistributing;         //use to mark whrther filr is distributing

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
int SendInfo(void* Info, int exception)
{
	char *info = Info;
	for(int i = 0; i < 100; ++i)
		//send to the client that exists and doesn't quit room
		if(conn[i].addr_len != -1 && conn[i].addr_len != 0 && conn[i].sock != exception){
			if(send (conn[i].sock, info , strlen(info) + 1, 0) == -1)
				printf("error occured, send to %s fail", conn[i].UserName);
			if(fileDistributing == 0)
				printf("---send <%s> to <%s> successfully!\n", info, conn[i].UserName);
		}	
	return 0;	
}

//this function sends the file of single client to the others
//File is the filename needed to be sent
int SendFile(connection_t* clientStruct)
{
	int size;
	int filesize;
	char buffer[1024];
	int len;
	fileDistributing = 1;

	//get the size of the file
	read(clientStruct->sock, &size, sizeof(int));
        read(clientStruct->sock, &filesize, sizeof(int));

	//send the file size to all the other clients
	//convert the int to string first
	//then send both the string length and string to the clients
	char filesizeString[20];
	char filesizeStringsize[2];
	sprintf(filesizeString, "%d", filesize);
	sprintf(filesizeStringsize, "%ld", strlen(filesizeString));
	SendInfo(filesizeStringsize, clientStruct->sock);
	SendInfo(filesizeString, clientStruct->sock);
	
	//send file in parts, reading and writing till the end
	for(int i=0; i < filesize/1024+1; ++i)
	{
		read(clientStruct->sock, &len, sizeof(int));
		read(clientStruct->sock, buffer, len);
		printf("receive %ld bytes\n", strlen(buffer));
		SendInfo(buffer, clientStruct->sock);
		printf("send part %d successful!\n", i + 1);
		bzero(buffer, BUFFER_SIZE);
	}
	
	//print success message and return
	printf("send all parts successful!\n");	
	fileDistributing = 0;
	return 0;
}

//This function deals with single client, aim to receive message from this client
//and then send them to another using SendInfo
void* Receive(void* clientStruct)
{

	connection_t* clientInfo = (connection_t *)clientStruct;
	while(1)
	{
		//if the server is sending file, don't receive useless information
		if(fileDistributing) continue;
		//read the message from the client
		char *Buffer;
		int messageLen = 0;
		read(clientInfo->sock, &messageLen, sizeof(int));  
		//if message is long enough, deal with it
		if(messageLen > 0)
		{
			Buffer = (char *)malloc((messageLen+1)*sizeof(char));
			read(clientInfo->sock, Buffer, messageLen); 
						
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
				SendInfo(quitMessage, -1);
				clientInfo->addr_len = -1;
				pthread_exit(&clientInfo->threadNumber);
			}
			else if ( Buffer[1] == 'f' && Buffer[2]  =='w')
			{	
				//send the file to the others
				char sign[] = "!!";
                                char file[] = " send you a file named as ";
				char fileMessage[50];
				char Filename[FILE_NAME_MAX_SIZE];
				fileMessage[0] = '\0';
				strcat(fileMessage, clientInfo->UserName);
				strcat(fileMessage, file);
				//read the file name  from buffer
				//send the file to the others
				for(int t = 4; t < messageLen-1; t++)
					Filename[t-4] = Buffer[t];
				Filename[messageLen-5]='\0';
				strcat(fileMessage, Filename);
				strcat(sign, fileMessage);
				SendInfo(sign, -1);
				SendFile(clientInfo);
			}
			else{
				//constitute the message
				char begin[] = " says";
				char messageDistribute[200];
				messageDistribute[0] = '\0';
				strcat(messageDistribute, clientInfo->UserName);
				strcat(messageDistribute, begin);
				strcat(messageDistribute, Buffer);
				SendInfo(messageDistribute, -1);
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
	char* buffer;
	int len;
	clientNumber = 0;   //initialize number of clients      
	long addr = 0;

	while(1){
		//waiting to be connected, can allow less than 100 clients to conect 
		if(clientNumber < 100)
		{
			conn[clientNumber].sock = accept(ServerSock, &conn[clientNumber].address, &conn[clientNumber].addr_len);
		}
		else
			break;

		//read the message length
		read(conn[clientNumber].sock, &len, sizeof(int));
		//only if the message is long enough to receive
		if (len > 0)
		{
			//neccessary information of a client
			addr = (long)((struct sockaddr_in *)&conn[clientNumber].address)->sin_addr.s_addr;
			buffer = (char *)malloc((len+1)*sizeof(char));
			buffer[len] = '\0';
			read(conn[clientNumber].sock, buffer, len);
			
			//save client's nick name
			strcpy(conn[clientNumber].UserName, buffer);
		
			//determine whether the UserName existed
			if(usernameExisted(conn[clientNumber].UserName, clientNumber))
				//reject connection and send reject information to the clients
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
				SendInfo(mesStart, -1);
				
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
	connection_t* connection;

	//create socket
	ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//bind the socket to a port
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	//if doesn't success, print fail message
	if (bind(ServerSock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "error: cannot bind socket to port %d\n", port);
		return -4;
	}

	//listen for connections
	//the second parameter marks max number of clients
	listen(ServerSock, 100);
	printf("the server is ready and listening\n");

	//creating a thread dealing with connections
	pthread_create(&thread, 0, process, (void *)connection);

	//keep this program working
	for(int i = 0; i < 100; ++i)
		sleep(10000);

	//close socket and thread
	pthread_detach(thread);
	close(ServerSock);
	return 0;
}
