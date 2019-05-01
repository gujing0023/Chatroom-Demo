    /* tcp-client.c */
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

int
main ()
{
  int sockfd, n;
  struct sockaddr_in serv, cli;
  char rec[1000];

  sockfd = socket (PF_INET, SOCK_STREAM, 0);

  bzero (&serv, sizeof (serv));

  serv.sin_family = PF_INET;
  serv.sin_port = htons (3690);
  serv.sin_addr.s_addr = inet_addr ("127.0.0.1");

  connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr));
  write (sockfd, "Hello, network!\n", 16);
  n = read (sockfd, rec, 1000);
  rec[n] = '\0';
  fputs (rec, stdout);
  close (sockfd);
  return 0;
}
