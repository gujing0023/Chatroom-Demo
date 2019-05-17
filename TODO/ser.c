#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <unistd.h>
pthread_t thread;
int ServerSock;

//每个用户的信息，包括socked、昵称等
typedef struct
{
	int sock;
	char UserName[16]; 
	struct sockaddr address;
	int addr_len;
} connection_t;


//TODO:
//该函数用于处理单个用户发送的信息，需设置循环以保证时刻处于工作状态
//注意！ 需处理是否退出聊天室！
//参数表示该用户的编号
void Receive(int clientnumber)
{

	//TODO：
	//SendInfo（"用户信息"）调用函数发送给所有用户
}


//TODO：
//该函数负责将用户的信息(包括发送内容，是否进入，是否退出等)发送给每一个在聊天室中的用户
//参数表示该用户的信息
//返回值为1表示发送成功，否则失败
int SendInfo(void* Info)
{

}

//TODO：
//该函数判断是否有重复
//返回值为1为有，否则为没有
int Judge(void* USERNAME)
{

}


//处理用户连接的线程函数
void * process(void * ptr)
{
	printf("The thread number is: %ld\n", thread); 
	char * buffer;
	int len;
	int clientNumber = 0;       //the number of the client that is connecting now
	connection_t  conn[10];     //this indicates that we only allow 10 clients to get into the chatroom
	long addr = 0;
	while(1){
		//等待用户连接
		if(clientNumber < 10)
		{
			conn[clientNumber].sock = accept(ServerSock, &conn[clientNumber].address, &conn[clientNumber].addr_len);
		}
		else
			continue;


		/* 读取用户发来的信息长度 */
		read(conn[clientNumber].sock, &len, sizeof(int));
		if (len > 0)
		{

			//读取用户的各种信息，包括地址、内容等
			addr = (long)((struct sockaddr_in *)&conn[clientNumber].address)->sin_addr.s_addr;
			buffer = (char *)malloc((len+1)*sizeof(char));
			buffer[len] = 0;
			read(conn[clientNumber].sock, buffer, len);


			//向用户发送连接成功的信息
			send (conn[clientNumber].sock, "You have entered the chatroom, Start CHATTING Now!\n", 51, 0);
			

			//保存用户的昵称并提示聊天室信息
			strcpy(conn[clientNumber].UserName, buffer);
		
			//TODO：
			//需要有一个函数判断是否用户名与之前的重复
			//返回值为1表明有，否则没有，有则直接拒绝登陆
			//拒绝登陆表明删除信息并不创建进程
			//Judge("UserName")

			printf("User <%s> has entered the Chatroom!\n", conn[clientNumber].UserName);
			printf("There are %d people in Chatroom now!\n",clientNumber+1);
			free(buffer);

			//TODO:
			//创建一个线程用于处理该用户发送的信息
			//pthread_create(&thread, 0, Receive, clientNumber);

		}
		clientNumber += 1;
		
	}
	pthread_exit(0);
}

int main(int argc, char ** argv){
	struct sockaddr_in address;
	int port = 8888;
	connection_t * connection;

	/*创建服务端的socket*/
	ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	/*将socket与端口绑定*/
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(ServerSock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "error: cannot bind socket to port %d\n", port);
		return -4;
	}


	/*listen，等待连接，第二个参数表示最大连接数*/
	listen(ServerSock, 10);
	printf("The server is ready and listening!\n");
	printf("Create a new thread!\n");


	//创建一个线程用于处理用户登陆信息
	pthread_create(&thread, 0, process, (void *)connection);


	//保证服务端线程持续运行
	for(int i = 0; i < 100; ++i)
		sleep((int)100000);


	//结束运行，聊天室永久关闭
	pthread_detach(thread);
	return 0;
}
