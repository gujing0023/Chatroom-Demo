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
int
main ()
{
  int sockfd, n;
  struct sockaddr_in serv, cli;
  char rec[1000];
  char send[80];
  while(fgets(send, sizeof(send), stdin))
  {
      //This is the length of the message you'd typed in
      int MessageSize = strlen(send);
      sockfd = socket (PF_INET, SOCK_STREAM, 0);

      bzero (&serv, sizeof (serv));
      

      //server information
      serv.sin_family = PF_INET;
      serv.sin_port = htons (8888);
      serv.sin_addr.s_addr = inet_addr ("127.0.0.1" /*local machine*/);


      connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr));
      //Send the exact message length and message context to the server
      write(sockfd, &MessageSize, sizeof(int));
      write (sockfd, send, sizeof(send));

      n = read (sockfd, rec, 1000);//n marks real length
      rec[n] = '\0';
      fputs (rec, stdout);
      send[0] = '\0';
      close(sockfd);
  }
  return 0;
}
