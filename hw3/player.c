#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"

int s;     // socket file descriptor

// Send "close" command to master
void close_player() {
  int len = send(s, "close", 5, 0);
  if (len != 5) {
    perror("send");
    exit(1);
  }
  close(s);
  exit(0);
}

// SIGINT (^c) handler
void intHandler() {
  close_player();
}

int main (int argc, char *argv[]) {
  int rc, port;
  unsigned long len;
  char host[HOSTNAME_LENGTH], str[HOSTNAME_LENGTH];
  struct hostent *hp;
  struct sockaddr_in sin;

  signal(SIGINT, intHandler);

  /* read host and port number from command line */
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <master-machine-name> <port-number>\n", argv[0]);
    exit(1);
  }

  /* fill in hostent struct */
  hp = gethostbyname(argv[1]);
  if (hp == NULL) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    exit(1);
  }
  port = atoi(argv[2]);

  /* create and connect to a socket */

  /* use address family INET and STREAMing sockets (TCP) */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("socket:");
    exit(s);
  }

  /* set up the address and port */
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);

  /* connect to socket at above addr and port */
  rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
  if (rc < 0) {
    perror("connect:");
    exit(rc);
  }

  // REQUIRED OUTPUT
  // TODO: get real number
  printf("Connected as player %d\n", 1);

  /* read a string from the terminal and send on socket */
  while (fgets(str, HOSTNAME_LENGTH, stdin) != NULL) {
    if (str[strlen(str)-1] == '\n') {
      str[strlen(str)-1] = '\0';
    }
    len = send(s, str, strlen(str), 0);
    if (len != strlen(str)) {
      perror("send");
      exit(1);
    }
  }

  close_player();

}
