#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

void * process(void * ptr)
{
	char * buffer;
	int len;
	connection_t * conn;
	long addr = 0;

	if (!ptr) pthread_exit(0); 
	conn = (connection_t *)ptr;
	//Create loop so that one client can communicate continuosly
	/* read length of message */
	read(conn->sock, &len, sizeof(int));
	printf("~~Message Length: %d~~", len - 1);
	if (len > 0)
	{
		//get the information from client
		addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
		buffer = (char *)malloc((len+1)*sizeof(char));
		buffer[len] = 0;
		/* read message */
		read(conn->sock, buffer, len);
		send (conn->sock, "Hello, Client! \n", 17, 0);
		/* print message content */
		printf("The client<%d.%d.%d.%d>said:", 
		(int)((addr      ) & 0xff),
		(int)((addr >>  8) & 0xff),
		(int)((addr >> 16) & 0xff),
		(int)((addr >> 24) & 0xff));
		printf("%s", buffer);
		free(buffer);
	}
	/* close socket and clean up */
	close(conn->sock);
	free(conn);
	pthread_exit(0);
}

int main(int argc, char ** argv)
{
	int sock = -1;
	struct sockaddr_in address;
	int port = 8888;
	connection_t * connection;
	pthread_t thread;

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "error: cannot bind socket to port %d\n", port);
		return -4;
	}

	/* listen on port */
	listen(sock, 10);
	printf("The server is ready and listening!\n");
	while (1)
	{
		/* accept incoming connections */
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->sock = accept(sock, &connection->address, &connection->addr_len);
		if (connection->sock <= 0)
		{
			free(connection);
		}
		else
		{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
		}
	}
	
	return 0;
}
