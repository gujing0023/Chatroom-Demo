    /* tcp-server.c */
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

int
main ()
{
  int sockfd, new_fd;
  struct sockaddr_in srvaddr, cliaddr;
  socklen_t sin_size;
  int n;
  char buf[80];

  sockfd = socket (AF_INET, SOCK_STREAM, 0);

  bzero (&srvaddr, sizeof (srvaddr));
  srvaddr.sin_family = AF_INET;
  srvaddr.sin_port = htons (3690);
  //srvaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  srvaddr.sin_addr.s_addr = inet_addr("104.238.136.201");
  bind (sockfd, (struct sockaddr *) &srvaddr, sizeof (struct sockaddr));
  listen (sockfd, 10);

  for (;;)
    {
      sin_size = sizeof (struct sockaddr_in);
      new_fd = accept (sockfd, (struct sockaddr *) &cliaddr, &sin_size);
      n = recvfrom(new_fd, buf, 80, 0, (struct sockaddr *)&cliaddr, &sin_size);
      printf ("server: got connection from %s, port %d: %s\n",
	      inet_ntoa (cliaddr.sin_addr), ntohs (cliaddr.sin_port), buf);
      send (new_fd, "Hello, Client! \n", 17, 0);
      close (new_fd);
    }
  close (sockfd);
  return 0;
}
