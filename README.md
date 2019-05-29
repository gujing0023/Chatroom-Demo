# 基于Socket编程的多人聊天室 <C语言实现>

### Multi-clients chat room based on Scoket Programming <C implementation>

> 项目已托管于github，项目地址：[github.com/gujing0023/Chatroom-Demo](https://github.com/gujing0023/Chatroom-Demo)

## 版本更迭

| 版本号                                                       | 完成时间  | 功能总述             |
| ------------------------------------------------------------ | --------- | -------------------- |
| [V1.0](https://github.com/gujing0023/Chatroom-Demo/tree/V1.0) | 2019/5/21 | 完成所有基础聊天功能 |
| [V1.1](https://github.com/gujing0023/Chatroom-Demo/tree/V1.1) | 2019/5/25 | 完成小文件收发(<5KB) |
| [V1.2](https://github.com/gujing0023/Chatroom-Demo/tree/V1.2) | 2019/5/29 | 完成函数实现解析文档 |

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
- 若服务器运行于一台远程服务器上，则需输入特定IP地址。比如说，在```示例图片1```中，我使用了一台位于美国迈阿密的Linux服务器，IP地址为```104.238.136.201```，直接输入该地址即可

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

### 用户端及服务器端函数总表

| 用户端函数        | 服务器端函数          |
| ----------------- | --------------------- |
| ```main```        | ```main```            |
| ```Send```        | ```process```         |
| ```Receive```     | ```usernameExisted``` |
| ```Sendfile```    | ```SendInfo```        |
| ```ReceiveFile``` | ```Receive```         |
|                   | ```SendFile```        |

### 用户端

#### main函数

用户端```main```函数除了创建```Socket```编程所需的基本信息外，还需要判断是否服务器拒绝连接，在本程序中，服务器拒绝连接的原因为昵称重复，关于关键语句及注释见下：

```C
...
//输入昵称并保存
Start: printf("Input Username: " );
...
//准备Socket等
...
//连接服务器
if(connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr)) == -1)
{
	...
}
//发送昵称给服务器端
write(sockfd, &MessageSize, sizeof(int));
write (sockfd, userName, sizeof(userName));
...
if(rec[0] == 'R')
{
    //被拒绝，重新输入昵称
	...
    GOTO: Start; 
    ...
}
else
{
    ...//没有被拒绝，创建接收及发送线程
	pthread_create(&threadSend, 0, Send, &sockfd);
	pthread_create(&threadReceive, 0, Receive, &sockfd);
    ...
}
...
//保证程序持续运行
for(int i = 0; i < 100; ++i)
	sleep(100000);
...
//关闭线程等
```

#### Send函数—用于发送基本信息

为了根据用户在终端输入的不同信息调用所有功能，定义了```Send ```函数。处理的信息为用户在对话框内输入的一切信息，包括想要传递给服务器的普通聊天信息、退出聊天室信息、发送文件标志信息和接收文件信息，**需注意：函数定义应满足线程函数调用要求**，函数原型：

```c
void* Send(void* Socket)
```

普通聊天信息格式为```:+空格+聊天内容```，保存在```sender```里，读取字符串长度后只需直接用两个```write```语句将其发送给服务器。

```c
write(*SockedCopy, &messageSize, sizeof(int));
write(*SockedCopy, sender, messageSize);
```

* 退出聊天室语句为```:q!```，只需判断之后直接退出。

```c
if(strcmp(sender, ":q!\n") == 0)
	exit(1);
```

* 发送文件语句格式为```:fw+空格+文件名```，屏幕提示之后输入待发送文件的绝对地址或与聊天室程序在同一个文件夹下的文件名并保存在```Filename```中。为了告诉服务器需要多少空间获取全部文件内容避免产生错误或遗漏内容，先打开文件读取文件大小，将其发送给服务器之后，再调用```Sendfile```函数发送文件内容。

```c
...
write(*SockedCopy, &intSize, sizeof(int));
write(*SockedCopy, &Filesize, sizeof(int));
Sendfile( Filename, SockedCopy );	
...
```

* 接收文件语句格式为```:fs+空格+保存文件地址```。将用户输入的地址从```sender```中截取出来并保存在```destination```之后调用```SendFile```函数接收文件内容：

```C
...
ReceiveFile(destination, *SockedCopy);
...
```

>  此处调用```ReceiveFile```函数用于接收文件，函数实现思路见下方

#### Receive函数—用于从服务器接收信息

```Receive```函数的主要功能是把其他用户通过服务器发送过来的聊天信息显示在当前用户的屏幕上，函数定义格式同样应**满足线程函数要求**，函数原型：

```C
void* Receive(void* Socked)
```

在接收文件时，为避免文件内容和聊天内容相混，设置一个```flag```名为```fileReading```，接收文件时为1，跳过信息的接收与显示：

```c
...
if(fileReading == 1) continue;
//循环内部判断开始接收文件，对fileReading赋值
if(Receiver[0] == '!' && Receiver[1] == '!') fileReading = 1;
...
```

其他情形下，直接接收信息并显示：

```c
...
reveiverEnd  = read (*SockedCopy, Receiver, 1000);
Receiver[reveiverEnd] = '\0';	
fputs(Receiver, stdout);
...
```

#### Sendfile函数—用于向服务器发送文件

当知道了是谁想要发送什么文件之后，就可以开始将文件内容发送给服务器。定义一个定长的字符串数组```buffer```，不断从文件中读取内容，利用```write```函数发送给服务器即可，函数原型：

```C
void Sendfile(char* Filename, void* Socked)
```

实现方式比较简单，打开文件后根据文件大小循环读取即可：

```c
...
fp = fopen(filename, "r");//打开文件
...
while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
{
    ...
	write(*SockedCopy, &length, sizeof(int));   //读取每段长度
	if(write(*SockedCopy, buffer, length) < 0)  //读取每段内容
	{
        ...
	}
}
...
```

#### ReceiveFile函数—用于从服务器接收文件

从终端得知保存文件的地址，从服务器得到文件大小之后，就能够使用循环接收文件内容写入目标文件之中。之前和之后需要改变```fileReading```的值，用于屏蔽聊天信息发送和接受对文件接收的干扰。函数原型：

```C
void ReceiveFile(char* dest, int Socket)
```

>  ```dest```为文件保存地址，```Socket```为服务器的```Socket```

* 打开文件并读取服务器端转发的文件大小信息，为循环接收创建条件：

```C
...
FILE *fp = fopen(dest, "w");//打开文件
...
int L1 = read(Socket, filesizeStringSize, 2);
int L2 = read(Socket, filesize, atoi(filesizeStringSize) + 1);//读取接收文件大小
...
```

* 接着就可以创建循环并不断接收文件信息了：

```C
...
while(i < filesizeInt/1024 + 1)
{	
	length = read(Socket, buffer, BUFFER_SIZE);//读取片段大小
	if(fwrite(buffer, sizeof(char), length - 2, fp) < length - 2)//写入文件
    {
		...
	}
}
...
```

### 服务器端

#### main函数

创建```Socket```并```listen```后创建```process进程```，实现方式较简单且为```Socket```编程基本内容，不赘述

#### 定义结构体connect_t—用于保存100个用户的基本信息

```C
typedef struct
{
	pthread_t threadNumber;//线程编号，在登录时开启，退出时关闭
	int sock;//Socket
	char UserName[16];//用户昵称
	struct sockaddr address;//地址
	int addr_len;//地址长度
} connection_t;
static connection_t conn[100];//100个用户一人一个
```

#### process函数—用于监听用户端的连接请求
当有一个新的用户加入聊天室时，```accept```函数结束阻塞。服务器完善起对应的结构体```conn```，包括保存IP地址、用户昵称、创建一个新的接收信息的线程。与此同时，还要调用```usernameExisted```函数判断昵称是否重复并反馈给新用户。在登陆成功之后，要告诉新用户和所有其他用户，**函数定义应满足线程函数要求**，函数原型：

```C
void * process(void * ptr)
```

```precess```函数需要处理的内容上方已经提到，必要语句与功能见下：

```c
...
while(1)
{

    ...
    //使用accept函数接收用户连接
	if(clientNumber < 100)
    	conn[clientNumber].sock = accept(...);
    ...
    //读取信息长度
	read(conn[clientNumber].sock, &len, sizeof(int));
    ...
	if (len > 0)
	{
		...
        //判断昵称是否重复
		if(usernameExisted(conn[clientNumber].UserName, clientNumber));
        ...
		else
		{
            ...
            //发送连接成功信息给用户
			send (conn[clientNumber].sock, ..., 51, 0);
			...
            //给所有用户发送消息    
			SendInfo(mesStart, -1);
            ...
            //创建一个新线程用于处理该用户发送的所有信息    
			pthread_create(...);
            ...
        }
	}
}
...
```

#### usernameExisted函数—用于判断用户昵称是否重复

新登录用户输入用户名之后，使用循环与除了自身之外的所有其他用户的用户名进行比较，函数原型：

```c
int usernameExisted(char userName[], int clientnumber)
```

> 两个参数依次为用户昵称以及用户号码，返回1表示存在，否则不存在，函数实现方式较简单，不赘述

#### SendInfo函数—用于将服务器接收到的信息转发给其他用户

当服务器接收到用户的聊天信息时需要把它分别发送给聊天室内的所有用户，同时把发送是否成功的信息显示在屏幕上。与用户端在发送文件时不能把收到的信息打印到屏幕上一样，服务器在分发文件时也不能在屏幕上显示成功与否的消息，所以设置一个flag，名为```fileDistributing```，分发文件时令其为1。函数原型：

```C
int SendInfo(void* Info, int exception)
```

> 参数依次为需要分发的内容，```exception```参数表示不需要发送的客户，通常为信息发送者自身

```c
for(int i = 0; i < 100; ++i)
{
		//发送给聊天室内所有存在并尚未离开的用户
		if(conn[i].addr_len != -1 && conn[i].addr_len != 0 && conn[i].sock != exception){
			if(send (conn[i].sock, info , strlen(info) + 1, 0) == -1)
				printf("error occured, send to %s fail", conn[i].UserName);
		//服务器不在分发文件时显示发送成功信息（服务器端）
			if(fileDistributing == 0)
				printf("---send <%s> to <%s> successfully!\n", info, conn[i].UserName);
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

#### SendFile函数—用于将接收到的文件信息发送给其他用户
文件发送与传统消息发送不同，且分段发送的机制无法在常规发送消息函数中实现，则需要定义一个新的函数用于文件发送，该函数在```Send```函数中被调用，函数原型如下：

```c
int SendFile(connection_t* clientStruct)
```

其中```clientStruct```即为发送文件的客户端，以```int```作为返回值起到了确定发送是否成功的作用

* 为了实现客户端与服务器端接收发送同步，我们首先需要告知所有客户端文件的大小

```C
...
read(clientStruct->sock, &size, sizeof(int));
read(clientStruct->sock, &filesize, sizeof(int));//服务器接收文件的大小
...
SendInfo(filesizeStringsize, clientStruct->sock);
SendInfo(filesizeString, clientStruct->sock);//将文件大小的信息发送给所有人
...
```

* 做好准备工作，下一步就可以将文件分段发送给客户端了

```c
...
for(int i=0; i < filesize/1024+1; ++i)
{
	//读取文件一个部分
	read(clientStruct->sock, &len, sizeof(int));
	read(clientStruct->sock, buffer, len);
	...
	//将此部分发送给所有人
	SendInfo(buffer, clientStruct->sock);
	...
}
...
```

>  以上即是所有函数的实现思路，关于部分函数细节，请参考附录或附件源代码。