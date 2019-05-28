# 基于Socket编程的多人聊天室 <C语言实现>

### Multi-clients chat room based on Scoket Programming <C implementation>

## 版本更迭

| 版本号                                                       | 完成时间  | 功能总述             |
| ------------------------------------------------------------ | --------- | -------------------- |
| [V1.0](https://github.com/gujing0023/Chatroom-Demo/tree/V1.0) | 2019/5/21 | 完成所有基础聊天功能 |
| [V1.1](https://github.com/gujing0023/Chatroom-Demo/tree/V1.1) | 2019/5/25 | 完成小文件收发(>5KB) |

## 功能

- 服务器端
  - [x] 开放接收客户连接，可部署于支持TCP/IP的任一台计算机中
  - [x] 接收来自客户端的连接并判断昵称是否重复
  - [x] 与客户端交互，如果有客户端登录、退出、提交聊天，将信息发给所有的客户端
  - [x] 支持最多100人同时聊天
  - [x] 接收来自客户端的文件并发送给其他客户端
- 客户端
  - [x] 根据输入IP地址连接特定服务器
  - [x] 在服务器中拥有唯一标识用户的昵称
  - [x] 发送信息、接收包括其他用户信息以及服务器信息
  - [x] 发送文件、接收包括其他用户文件

> 聊天功能展示

![demo-image](https://github.com/gujing0023/Chatroom-Demo/blob/master/demo/demoChat.png)

> 收发文件功能展示

![demo-image](https://github.com/gujing0023/Chatroom-Demo/blob/master/demo/demoFile.jpg)

## 食用方法

clone项目至本地，使用

```shell
git clone "https://github.com/gujing0023/Chatroom-Demo.git"
```

进入```src```文件夹，使用```make```命令编译```cli.c```以及```ser.c```文件生成服务器端和客户端程序，并使用```make copy```命令创建多个客户端(可选)

```shell
cd Chatroom-Demo/src
make
make copy
```

将生成的```ser```作为服务器端程序置于一台你想作为服务器端的计算机上（**需明确服务器端IP地址**），将```cli1```，```cli2```，```cli3```等客户端文件置于你想作为客户端的计算机们上

服务器端运行等待连接

```shell
./ser
the server is ready and listening
```

客户端运行，输入服务器地址：

- 若服务器与客户端都运行于本地，直接回车即可
- 若服务器运行于一台远程服务器上，则需输入特定IP地址。在示例中，我使用了一台位于美国迈阿密的Linux服务器，IP地址为```104.238.136.201```，直接输入该地址即可

输入昵称并回车，如返回

```shell
You have entered the chatroom, Start CHATTING Now!
```

说明连接成功，可以进行聊天了！！下表列出了客户端可以进行的操作

| 输入格式                                                 | 示例                         | 效果                                                 |
| -------------------------------------------------------- | ---------------------------- | ---------------------------------------------------- |
| ":" + “<空格>” + "你想说的话"                            | : Hello！I‘m llht！          | 所有聊天室中的客户均可收到该消息                     |
| ":" + "q!"                                               | :q!                          | 结束客户程序并退出，所有客户收到你退出的消息         |
| ":" + "fw" + "<空格>" + "文件名" + "<回车>" + "文件地址" | :fw Makefile<回车>./Makefile | 发送该文件给其他客户端<不包括本客户端>               |
| ":" + "fs" + "<空格>" + "保存地址及文件名"               | :fs ./fileRec/Makefile       | 保存文件至该地址，只有当服务器提示接收到文件才可使用 |
> 客户端收到文件接受提示时，只能使用<:fs>命令进行文件保存，不可进行其他操作！
## 实现方法及部分细节

### 用户端

#### Send函数—用于发送基本信息

为了根据用户在终端输入的不同信息调用所有功能，定义了```Send ```函数。处理的信息为用户在对话框内输入的一切信息，包括想要传递给服务器的普通聊天信息、退出聊天室信息、发送文件标志信息和接收文件信息。

普通聊天信息格式为```:+空格+聊天内容```，保存在sander里，读取字符串长度后只需直接用两个write语句将其发送给服务器。

```c
write(*SockedCopy, &messageSize, sizeof(int));
write(*SockedCopy, sender, messageSize);
```

退出聊天室语句为```:q!```，只需判断之后直接退出。

```c
if(strcmp(sender, ":q!\n") == 0)
exit(1);
```

发送文件语句格式为```:fw+空格+文件名```，屏幕提示之后输入待发送文件的绝对地址或与聊天室程序在同一个文件夹下的文件名并保存在```Filename```中。为了告诉服务器需要多少空间获取全部文件内容避免产生错误或遗漏内容，先打开文件读取文件大小，将其发送给服务器之后，再调用```Sendfile```函数发送文件内容。

```c
FILE *fp=fopen(Filename, "r");
write(*SockedCopy, &intSize, sizeof(int));
write(*SockedCopy, &Filesize, sizeof(int));
Sendfile( Filename, SockedCopy );	
```

接收文件语句格式为```:fs+空格+保存文件地址```。将用户输入的地址从```sender```中截取出来并保存在```destination```之后调用```SendFile```函数接收文件内容。

```c
for(int i = 4; i <  strlen(sender) - 1; ++i)
destination[i - 4] = sender[i];
destination[strlen(sender) - 5] = '\0';
ReceiveFile(destination, *SockedCopy);
```

#### Receive函数—用于从服务器接收信息

```Receive```函数的主要功能是把其他用户通过服务器发送过来的聊天信息显示在当前用户的屏幕上。但是，在接收文件时，为避免文件内容和聊天内容相混，设置一个```flag```名为```fileReading```，接收文件时为1，跳过信息的接收与显示。

```c
if(fileReading == 1) continue;
//循环内部判断开始接收文件，对fileReading赋值
if(Receiver[0] == '!' && Receiver[1] == '!') fileReading = 1;
```

其他情形下，直接接收信息并显示。

```c
reveiverEnd  = read (*SockedCopy, Receiver, 1000);
Receiver[reveiverEnd] = '\0';	
fputs(Receiver, stdout);
```

#### Sendfile函数—用于向服务器发送文件

当知道了是谁想要发送什么文件之后，就可以开始将文件内容发送给服务器。定义一个定长的字符串数组```buffer```，不断从文件中读取内容，利用```write```函数发送给服务器即可。

```c
void* Sendfile(char* Filename, void* Socked)
{
	...
	fp = fopen(filename, "r");
//找不到文件
	if(NULL == fp)
	{
		printf("File:%s Not Found\n", filename);
	}
	else
	{
//循环不断读取文件内容并发送给服务器
		while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			write(*SockedCopy, &length, sizeof(int));
//中途有任何错误就报错
			if(write(*SockedCopy, buffer, length) < 0)
			{
				printf("Upload file:%s Failed.\n", filename);
				break;
			}
			bzero(buffer, BUFFER_SIZE);
		}
	}
//关闭文件并表示传送成功
	fclose(fp);
	printf("File:%s Upload Successfully!\n", filename);
	
}
```

#### ReceiveFile函数—用于从服务器接收文件

```c
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
```

### 服务器端

#### 定义结构体connect_t—用于保存100个用户的基本信息

```c
typedef struct
{
	pthread_t threadNumber; //线程编号，在登录时开启，退出时关闭
	int sock; //sock字符
	char UserName[16]; //用户昵称
	struct sockaddr address; //地址
	int addr_len; //地址长度
} connection_t;
static connection_t conn[100]; //100个用户一人一个
```

#### process函数—用于监听用户端的连接请求

```c
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
```

#### usernameExisted函数—用于判断用户昵称是否重复

```c
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
```

#### SendInfo函数—用于将服务器接收到的信息转发给其他用户

```c
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
```

#### Receive函数—用于从用户端接收信息

为了从用户端接收消息并及时将消息传送给聊天室中的其他人，定义了```Receive```函数处理每个用户的信息接收，参数```clientStruct```为该用户的各类信息：

```c
void* Receive(void* clientStruct)
```

使用```while(1)```循环保证用户发送的信息可以持续接收，但考虑到特定用户发送文件的同时，若自主输入聊天信息，则会对服务器的文件接收进行干扰，设置一个flag，即```fileDistributing```，在进行文件发送的时候，不接收该用户的聊天信息，即：

```c
if (fileDistributing) continue;
```

使用```socket```编程中的```read()```函数接收用户信息，并根据用户信息内容判断用户需求，有关用户输入字符与用户需求的关系，参考上文**食用方法**中的表格，使用```if else```语句结构对特定需求做出判断，即：

```c
if( Buffer[1] == 'q' && Buffer[2] == '!' )
{
//用户想退出聊天室
    ...
    SendInfo(quitMessage, -1);//组装消息后发送给其他客户端
    ...
    pthread_exit(&clientInfo->threadNumber);//退出支持该用户的当前进程
}
else if ( Buffer[1] == 'f' && Buffer[2]  =='w')
{
//用户想发送文件到聊天室中
    ...
    SendInfo(sign, -1);//组装消息后发送给其他客户端提示需要接收了！
	SendFile(clientInfo);//使用SendFile函数将文件发送给其他客户端
}
else
{
//用户仅仅是想发送一条聊天的话而已
    ...
    SendInfo(messageDistribute, -1);//组装信息后发送给其他人
}
```

以上即是```Receive```函数的实现思路，关于部分函数细节，请参考附录或附件源代码。

#### SendFile函数—用于将接收到的文件信息发送给其他用户

```c
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

	for(int i=0; i < filesize/1024+1; ++i)
	{
		read(clientStruct->sock, &len, sizeof(int));
		read(clientStruct->sock, buffer, len);
		printf("receive %ld bytes\n", strlen(buffer));
		SendInfo(buffer, clientStruct->sock);
		printf("send part %d successful!\n", i + 1);
		bzero(buffer, BUFFER_SIZE);
	}
	printf("send all parts successful!\n");	
	fileDistributing = 0;
	return 0;
}
```


