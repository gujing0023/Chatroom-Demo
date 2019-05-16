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


//TODO:
//此函数用于发送消息到聊天室（需设置循环，以保证每次用户想说话都能说出去）
//参数为socket
//返回值为1表示发送成功，否则发送失败
int Send(void* )
{

}

//TODO:
//此函数用于接收服务器信息（定时，每1秒接收一次）
//参数为socket
//返回值为1表示接收成功，否则接收失败
int Receive(void* )
{

}

int main ()
{
	int sockfd, n;
	struct sockaddr_in serv, cli;
	char rec[1000];
	char send[80];

	//输入用户昵称，即UserName
	printf("Input Username:" );
	fgets(send, sizeof(send), stdin);
	send[strlen(send) - 1] = '\0';
 	int MessageSize = strlen(send);

	//创建本机socket
 	sockfd = socket (PF_INET, SOCK_STREAM, 0);

	//创建服务器信息，即本机
 	bzero (&serv, sizeof (serv));
	serv.sin_family = PF_INET;
	serv.sin_port = htons (8888);
 	serv.sin_addr.s_addr = inet_addr ("127.0.0.1" /*local machine*/);

	//连接服务器
	connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr));
	
	//将昵称发送给服务器
	write(sockfd, &MessageSize, sizeof(int));
 	write (sockfd, send, sizeof(send));

	//收到连接成功的信息
	n = read (sockfd, rec, 1000);//n marks real length
 	rec[n] = '\0';	
	fputs(rec, stdout);
	
	send[0] = '\0';

	//TODO: 
	//使用thread_create打开一个线程，这个线程负责当前client的聊天信息发送 
	//thread_create(&thread, 0, Send, sockfd)
		

	//TODO:
	// 使用thread_create打开一个线程，这个线程负责当前client接收聊天信息
	//thread_create(&thread, 0, Receive, sockfd)
		
	//关闭socked
	close(sockfd);
	return 0;
}
