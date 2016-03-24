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

// network state
int listen_socket;  // socket file descriptor

// game state
int num_players, hops, players_connected;

// SIGINT (^c) handler
void intHandler() {
  close(listen_socket);
  exit(0);
}

/*
 * Blocks until num_player connections are created and closed.
 * Expects listen_address and listen_socket to be initialized.
 * Initializes the player state for master.
 */
void accept_checkin() {
  char buffer[512];
  socklen_t len;
  int num_bytes;
  int accept_fd;
  struct sockaddr_in incoming;

  len = sizeof(incoming);
  accept_fd = accept(listen_socket, (struct sockaddr *)&incoming, &len);        // block until a client connects to the server, then return new file descriptor
  if ( accept_fd < 0 ) {
    perror("bind");
    exit(accept_fd);
  }

  char host[HOSTNAME_LENGTH], service[20];
  getnameinfo((struct sockaddr *)&incoming, sizeof incoming, host, sizeof host, service, sizeof service, 0);

  // REQUIRED output
  printf("player %d is on %s\n", players_connected, host);
  players_connected++;

  /* read and print strings sent over the connection */
  // http://www.beej.us/guide/bgnet/output/html/singlepage/bgnet.html#sendrecv
  while (1) {
    bzero(buffer, 512);
    if ((num_bytes = recv(accept_fd, buffer, 512, 0)) < 0) {   // block until input is read from socket
      perror("recv");
      exit(1);
    }
    DEBUG_PRINT("num_bytes = %d\n", num_bytes);

    if (num_bytes == 0) {
      close(accept_fd);
      printf(">> Connection closed\n");
      return;
    }

    buffer[num_bytes] = '\0';
    char *token = strtok(buffer, "\n");
    while (token) {
      if (!strcmp("close", token)) {
        close(accept_fd);
        printf(">> Connection closed\n");
        return;
      } else {
        printf("%s\n", token);
      }
      token = strtok(NULL, "\n");
    }

  }
}

int main (int argc, char *argv[]) {
  int listen_port;

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

  listen_socket = setup_listener(&listen_port);

  // REQUIRED OUTPUT
  printf("Potato Master on %s\n", gethostcanonicalname("localhost", 9999));   // This is the “official” name of the host.
  printf("Players = %d\n", num_players);
  printf("Hops = %d\n", hops);

  /* accept connections */
  while (players_connected < num_players) {
    accept_checkin();
  }

  int first_player = randr(0, num_players-1);

  // REQUIRED OUTPUT
  printf("All players present, sending potato to player %d\n", first_player);

  exit(0);
}
