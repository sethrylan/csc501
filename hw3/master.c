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


  read_message(accept_fd, buffer, 512);

  char *token = strtok(buffer, "\n");
  while (token) {
    printf("%s\n", token);
    token = strtok(NULL, "\n");
  }
  printf(">> Connection closed\n");

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

  //TODO: int seed = 42;
  // srand(seed);
  int first_player = randr(0, num_players-1);

  // REQUIRED OUTPUT
  printf("All players present, sending potato to player %d\n", first_player);

  exit(0);
}
