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
extern const char *__progname;

// network state
int listen_socket;        // socket file descriptor
int listen_port;
struct sockaddr_in listen_address;

// game state
int num_players, hops, players_connected;   // arguments from command line


// SIGINT (^c) handler
void intHandler() {
  close(listen_socket);
  exit(0);
}

struct hostent *gethostent() {
  struct hostent *hp;
  char host[64];

  /* fill in hostent struct for self */
  gethostname(host, sizeof host);
  hp = gethostbyname(host);
  DEBUG_PRINT("hp->h_addr_list[0] = %s\n", hp->h_addr_list[0]);
  if (hp == NULL) {
    if (h_errno == HOST_NOT_FOUND) {
      fprintf(stderr, "%s: host not found (%s)\n", __progname, host);
      exit(1);
    } else {
      perror(__progname);
      exit(1);
    }
  }
  return hp;
}

/* Open a socket for listening
 *  1. create socket
 *  2. bind it to an address/port
 *  3. listen
 *
 * Initializes listen_address variable
 * Returns file descriptor for listen socket, or -1 if not able to listen
 *
 */
int setup_listener(int listen_port) {
  int retval;

  /* use address family INET and STREAMing sockets (TCP) */
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket:");
    exit(socket_fd);
  }

  /* set up the address and port */
  bzero((char *) &listen_address, sizeof(listen_address));   // set all values in address buffer to zero
  listen_address.sin_family = AF_INET;                       // "the correct thing to do is to use AF_INET in your struct sockaddr_in" (http://beej.us/net2/html/syscalls.html)
  listen_address.sin_port = htons(listen_port);              // convert port to network byte order
  listen_address.sin_addr.s_addr = htonl(INADDR_ANY);        // IP address of the host. For server code, this will always be the IP address of the machine on which the server is running.
  memset(&(listen_address.sin_zero), '\0', 8);
  // memcpy(&listen_address.sin_addr, hp->h_addr_list[0], hp->h_length);  // alternative to INADDR_ANY, which doesn't trigger firewall protection on OSX

  // bind socket s to address sin
  // if bind() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  retval = bind(socket_fd, (struct sockaddr *)&listen_address, sizeof(listen_address));
  if (retval < 0) {
    perror("bind:");
    exit(retval);
  }

  // if listen() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  retval = listen(socket_fd, 5);   // second argument is size of the backlog queue (number of connections that can be waiting while the process is handling a particular connection)
  if (retval < 0) {
    perror("listen:");
    exit(retval);
  }

  return socket_fd;
}

/*
 * Blocks until num_player connections are created and closed.
 * Expects listen_address and listen_socket to be initialized.
 * Initializes the player state for master.
 */
void accept_checkins() {
  char buffer[512];
  socklen_t len;
  int p;
  struct hostent *ihp;
  struct sockaddr_in incoming;

  len = sizeof(listen_address);
  p = accept(listen_socket, (struct sockaddr *)&incoming, &len);        // block until a client connects to the server, then return new file descriptor
  if ( p < 0 ) {
    perror("bind:");
    exit(p);
  }
  ihp = gethostbyaddr((char *)&incoming.sin_addr, sizeof(struct in_addr), AF_INET);

  // REQUIRED output
  printf("player %d is on %s\n", players_connected, ihp->h_name);
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
  signal(SIGINT, intHandler);

  players_connected = 0;

  /* read port number from command line */
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <port-number> <number-of-players> <hops>\n", argv[0]);
    exit(1);
  }
  listen_port = atoi(argv[1]);
  num_players = atoi(argv[2]);
  hops = atoi(argv[3]);

  if (num_players < 1 || hops < 0) {
    fprintf(stderr, "Usage: %s <number-of-players> <hops>\n", argv[0]);
    fprintf(stderr, "‹number-of-players› must be > 0 and ‹hops› must be ≥ 0\n");
    exit(1);
  }

  struct hostent *host_listener = gethostent();
  listen_socket = setup_listener(listen_port);

  // REQUIRED OUTPUT
  printf("Potato Master on %s\n", host_listener->h_name);
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
