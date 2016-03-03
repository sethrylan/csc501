/******************************************************************************
 *
 *  File Name........: listen.c
 *
 *  Description......:
 *	Creates a program that establishes a passive socket.  The socket 
 *  accepts connection from the speak.c program.  While the connection is
 *  open, listen will print the characters that come down the socket.
 *
 *  Listen takes a single argument, the port number of the socket.  Choose
 *  a number that isn't assigned.  Invoke the speak.c program with the 
 *  same port number.
 *
 *  Revision History.:
 *
 *  When	Who         What
 *  09/02/96    vin         Created
 *
 *****************************************************************************/

/*........................ Include Files ....................................*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

main (int argc, char *argv[])
{
  char buf[512];
  char host[64];
  int s, p, fp, rc, len, port;
  struct hostent *hp, *ihp;
  struct sockaddr_in sin, incoming;

  /* read port number from command line */
  if ( argc < 2 ) {
    fprintf(stderr, "Usage: %s <port-number>\n", argv[0]);
    exit(1);
  }
  port = atoi(argv[1]);

  /* fill in hostent struct for self */
  gethostname(host, sizeof host);
  hp = gethostbyname(host);
  if ( hp == NULL ) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    exit(1);
  }

  /* open a socket for listening
   * 4 steps:
   *	1. create socket
   *	2. bind it to an address/port
   *	3. listen
   *	4. accept a connection
   */

  /* use address family INET and STREAMing sockets (TCP) */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }

  /* set up the address and port */
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* bind socket s to address sin */
  rc = bind(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("bind:");
    exit(rc);
  }

  rc = listen(s, 5);
  if ( rc < 0 ) {
    perror("listen:");
    exit(rc);
  }

  /* accept connections */
  while (1) {
    len = sizeof(sin);
    p = accept(s, (struct sockaddr *)&incoming, &len);
    if ( p < 0 ) {
      perror("bind:");
      exit(rc);
    }
    ihp = gethostbyaddr((char *)&incoming.sin_addr, 
			sizeof(struct in_addr), AF_INET);
    printf(">> Connected to %s\n", ihp->h_name);
 
    /* read and print strings sent over the connection */
    while ( 1 ) {
      len = recv(p, buf, 32, 0);
      if ( len < 0 ) {
	perror("recv");
	exit(1);
      }
      buf[len] = '\0';
      if ( !strcmp("close", buf) )
	break;
      else
	printf("%s\n", buf);
    }
    close(p);
    printf(">> Connection closed\n");
  }
  close(s);
  exit(0);
}

/*........................ end of listen.c ..................................*/
