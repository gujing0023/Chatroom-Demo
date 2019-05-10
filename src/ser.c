/*************************************************************************
	> File Name: ser.c
	> Author: battle
	> Mail: batbattle@163.com 
	> Created Time: 2018年06月02日 星期六 11时21分02秒
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
#define BACKLOG 10
#define MAXDATASIZE  2048

int main(int argc, char *argv[])
{
    int listenfd;

    //创建一个socket描述符，此描述符仅是本主机上的一个普通文件描述符而已
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("listenfd=%d\n", listenfd);

    //定义一个结构体变量servaddr，用来记录给定的IP和port信息，为bind函数做准备
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT); //把端口转化为网络字节序，即大端模式
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    //把“本地含义的描述符”绑定到一个IP和Port上，此时这个socket才具备对外连接的能力
    bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    //创建一个监听队列，用来保存用户的请求连接信息（ip、port、protocol)
    listen(listenfd, BACKLOG);

    printf("======bind success,waiting for client's request======\n");
    //让操作系统回填client的连接信息（ip、port、protocol）
    struct sockaddr_in peeraddr;
    socklen_t peer_len = sizeof(peeraddr);
    int connfd;

    while(1)
    {
        //accept函数从listen函数维护的监听队列里取一个客户连接请求处理
        connfd = accept(listenfd, (struct sockaddr*)&peeraddr, &peer_len);
        printf("\n=====================客户端链接成功=====================\n");
        printf("IP = %s:PORT = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        char buf[MAXDATASIZE];
        while(1)
        {
            memset(buf, '\0', MAXDATASIZE/sizeof (char));
            int recv_length = recv(connfd, buf, MAXDATASIZE/sizeof (char), 0);
            if(recv_length == 0)
            {
                printf("client has closed!\n");
                break;
            }
            printf("client say[%d]: ", recv_length);
            fputs(buf, stdout);
            memset(buf, '\0', MAXDATASIZE/sizeof (char));
            printf("input: ");
            fgets(buf, sizeof(buf), stdin);
            send(connfd, buf, recv_length, 0);
        }
        close(connfd);
        close(listenfd);
        return 0;
	}
}
