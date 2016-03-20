#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"

extern int h_errno;

int listen_socket;        // socket file descriptor
int rc;
int port, num_players, hops;
struct sockaddr_in address;
int players_connected;

// SIGINT (^c) handler
void intHandler() {
  close(listen_socket);
  exit(0);
}

void accept_checkins() {
  char buffer[512];
  socklen_t len;
  int p;
  struct hostent *ihp;
  struct sockaddr_in incoming;

  len = sizeof(address);
  p = accept(listen_socket, (struct sockaddr *)&incoming, &len);        // block until a client connects to the server, then return new file descriptor
  if ( p < 0 ) {
    perror("bind:");
    exit(rc);
  }
  ihp = gethostbyaddr((char *)&incoming.sin_addr, sizeof(struct in_addr), AF_INET);

  // REQUIRED output
  printf("player %d is on %s\n", 1, ihp->h_name);
  players_connected++;

  /* read and print strings sent over the connection */
  while (1) {
    bzero(buffer, 512);
    len = read(p, buffer, 512);   // block until input is read from socket
    // DEBUG_PRINT("len = %d\n", len);
    // if ( len < 0 ) {
    //   perror("recv");
    //   exit(1);
    // }
    buffer[len] = '\0';
    if ( !strcmp("close", buffer) ) {
      break;
    } else if (len > 0) {
      printf("%s\n", buffer);
    }
  }
  close(p);
  printf(">> Connection closed\n");
}

int main (int argc, char *argv[]) {
  char host[64];
  struct hostent *hp;

  signal(SIGINT, intHandler);

  players_connected = 0;

  /* read port number from command line */
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <port-number> <number-of-players> <hops>\n", argv[0]);
    exit(1);
  }
  port = atoi(argv[1]);
  num_players = atoi(argv[2]);
  hops = atoi(argv[3]);

  if (num_players < 1 || hops < 0) {
    fprintf(stderr, "Usage: %s <number-of-players> <hops>\n", argv[0]);
    fprintf(stderr, "‹number-of-players› must be > 0 and ‹hops› must be ≥ 0\n");
    exit(1);
  }

  /* fill in hostent struct for self */
  gethostname(host, sizeof host);
  hp = gethostbyname(host);
  if (hp == NULL) {
    if (h_errno == HOST_NOT_FOUND) {
      fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
      exit(1);
    } else {
      herror(argv[0]);
      exit(1);
    }
  }

  /* open a socket for listening
   *  1. create socket
   *  2. bind it to an address/port
   *  3. listen
   *  4. accept a connection
   */

  /* use address family INET and STREAMing sockets (TCP) */
  listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket < 0) {
    perror("socket:");
    exit(listen_socket);
  }

  /* set up the address and port */
  bzero((char *) &address, sizeof(address));   // set all values in address buffer to zero
  address.sin_family = AF_INET;                // "the correct thing to do is to use AF_INET in your struct sockaddr_in" (http://beej.us/net2/html/syscalls.html_
  address.sin_port = htons(port);              // convert port to network byte order
  DEBUG_PRINT("hp->h_addr_list[0] = %s\n", hp->h_addr_list[0]);
  memcpy(&address.sin_addr, hp->h_addr_list[0], hp->h_length);

  // adding INADDR_ANY prompts for network listen
  // address.sin_addr.s_addr = INADDR_ANY;;    // IP address of the host. For server code, this will always be the IP address of the machine on which the server is running.

  // bind socket s to address sin
  rc = bind(listen_socket, (struct sockaddr *)&address, sizeof(address));
  if ( rc < 0 ) {
    perror("bind:");
    exit(rc);
  }

  rc = listen(listen_socket, 5);        // second argument is size of the backlog queue (number of connections that can be waiting while the process is handling a particular connection)
  if ( rc < 0 ) {
    perror("listen:");
    exit(rc);
  }

  // REQUIRED OUTPUT
  printf("Potato Master on %s\n", host);
  printf("Players = %d\n", num_players);
  printf("Hops = %d\n", hops);

  /* accept connections */
  while (players_connected < num_players) {
    accept_checkins();
  }

  int first_player = randr(0, num_players-1);

  // REQUIRED OUTPUT
  printf("All players present, sending potato to player %d\n", first_player);

  exit(0);
}
