/*me: cli.c
> Author: battle
> Mail: batbattle@163.com 
> Created Time: 2018年06月02日 星期六 10时55分37秒
 ************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define PORT 8888
#define MAXDATASIZE 2048
int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Please inpu\n");
		exit(1);
	}
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	const char *server_ip = argv[1]; //从命令行获取输入的ip地址
	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	inet_pton(AF_INET, server_ip, &serveraddr.sin_addr);
       	connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
       	printf("=================server connection success!=====================\n");
	char buf[MAXDATASIZE];
	memset(buf, 0, sizeof(buf));
	printf("input: ");
	while(fgets(buf, sizeof(buf), stdin) != NULL && (strcmp(buf, "quit")))
	{
		send(sockfd, buf, sizeof(buf), 0);
		memset(buf, 0, sizeof(buf));
		recv(sockfd, buf, sizeof(buf), 0);
		printf("server say: ");
		fputs(buf, stdout);
		memset(buf, 0, sizeof(buf));
		printf("input: ");
	}
	printf("client will be closed, see you next time.\n");
	close(sockfd);
	return 0;
}
