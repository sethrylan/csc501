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
player* players;

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

  read_message(accept_fd, buffer, 512);

  char *token = strtok(buffer, "\n");
  while (token) {
    DEBUG_PRINT("%s\n", token);
    if (begins_with(token, "CONNECT:")) {
      char *port_string = malloc(10);
      strncpy(port_string, token + strlen("CONNECT:"), strlen(token) - strlen("CONNECT:"));
      DEBUG_PRINT("Adding player(%s:%s)\n", host, port_string);
      struct addrinfo *player_listner = gethostaddrinfo(host, atoi(port_string));
      players[players_connected].address_info = player_listner;
      players[players_connected].player_id = players_connected;
      // strcpy(players[players_connected].address, host);
      // players[players_connected].listen_port = token[8];
    }
    token = strtok(NULL, "\n");
  }
  DEBUG_PRINT(">> Connection closed\n");
  players_connected++;
}

void send_message(int socket_fd, char* message) {
  char str[100];
  unsigned long len;
  len = send(socket_fd, message, strlen(message), 0);
  DEBUG_PRINT("len = %lu\n", len);
  if (len != strlen(str)) {
    perror("send");
    exit(1);
  }
}

void send_info_to_player(int player_number) {
  struct addrinfo *address = players[player_number].address_info;
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("socket:");
    exit(s);
  }

  // connect to socket at addr and port
  // if connect() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  int retval = connect(s, address->ai_addr, address->ai_addrlen);
  if (retval < 0) {
    perror("connect");
    exit(retval);
  }

  send_message(s, "YOUARE:1\n");
  send_message(s, "L:66666\n");
  send_message(s, "R:55555\n");

  close(s);
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

  players = malloc(sizeof(player) * num_players);

  listen_socket = setup_listener(&listen_port);

  // REQUIRED OUTPUT
  printf("Potato Master on %s\n", gethostcanonicalname("localhost", 9999));   // This is the “official” name of the host.
  printf("Players = %d\n", num_players);
  printf("Hops = %d\n", hops);

  /* accept connections */
  while (players_connected < num_players) {
    accept_checkin();
  }

  int i;
  for (i = 0; i < num_players; i++) {
    send_info_to_player(i);
  }

  int seed = 42;
  srand(seed);
  int first_player = randr(0, num_players-1);

  // REQUIRED OUTPUT
  printf("All players present, sending potato to player %d\n", first_player);

  free(players);
  exit(0);
}
