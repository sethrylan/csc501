/******************************************************************************
 *
 *  File Name........: speak.c
 *
 *  Description......:
 *	Create a process that talks to the listen.c program.  After 
 *  connecting, each line of input is sent to listen.
 *
 *  This program takes two arguments.  The first is the host name on which
 *  the listen process is running. (Note: listen must be started first.)
 *  The second is the port number on which listen is accepting connections.
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

#define LEN	64
 
main (int argc, char *argv[])
{
  int s, rc, len, port;
  char host[LEN], str[LEN];
  struct hostent *hp;
  struct sockaddr_in sin;

  /* read host and port number from command line */
  if ( argc != 3 ) {
    fprintf(stderr, "Usage: %s <host-name> <port-number>\n", argv[0]);
    exit(1);
  }
  
  /* fill in hostent struct */
  hp = gethostbyname(argv[1]); 
  if ( hp == NULL ) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    exit(1);
  }
  port = atoi(argv[2]);

  /* create and connect to a socket */

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
  
  /* connect to socket at above addr and port */
  rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("connect:");
    exit(rc);
  }

  /* read a string from the terminal and send on socket */
  while ( fgets(str, LEN, stdin) != NULL ) {
    if (str[strlen(str)-1] == '\n')
      str[strlen(str)-1] = '\0';
    len = send(s, str, strlen(str), 0);
    if ( len != strlen(str) ) {
      perror("send");
      exit(1);
    }
  }

  /* when finished sending, tell host you are closing and close */
  len = send(s, "close", 5, 0);
  if (len != 5) {
    perror("send");
    exit(1);
  }
  close(s);
  exit(0);
}

/*........................ end of speak.c ...................................*/
