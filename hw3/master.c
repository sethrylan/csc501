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
struct addrinfo** players;

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
  struct sockaddr_in incoming;

  socklen_t len = sizeof(incoming);
  int accept_fd = accept(listen_socket, (struct sockaddr *)&incoming, &len);        // block until a client connects to the server, then return new file descriptor
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
    if (begins_with(token, CONNECT_PREFIX)) {
      char *port_string = malloc(10);
      strncpy(port_string, token + strlen(CONNECT_PREFIX), strlen(token) - strlen(CONNECT_PREFIX));
      DEBUG_PRINT("Adding player #%d as %s:%s\n", players_connected, host, port_string);
      struct addrinfo *player_listener = gethostaddrinfo(host, atoi(port_string));
      players[players_connected] = player_listener;
    }
    token = strtok(NULL, "\n");
  }
  DEBUG_PRINT(">> Checkin finished\n");
  players_connected++;
}

void send_info_to_player(int player_number) {
  char host[HOSTNAME_LENGTH], service[20], str[200], left_address_str[INET_ADDRSTRLEN], right_address_str[INET_ADDRSTRLEN];
  struct addrinfo *player_address = players[player_number];
  struct addrinfo *left_address   = players[(player_number-1)%num_players];
  struct addrinfo *right_address  = players[(player_number+1)%num_players];

  DEBUG_PRINT("send_info_to_player(%d)\n", player_number);

  // compose left_address_str for left neighbor
  getnameinfo(left_address->ai_addr, left_address->ai_addrlen, host, sizeof host, service, sizeof service, 0);
  sprintf(left_address_str, "%s:%s", host, service);

  // compose right_address_str for right neighbor
  getnameinfo(right_address->ai_addr, right_address->ai_addrlen, host, sizeof host, service, sizeof service, 0);
  sprintf(right_address_str, "%s:%s", host, service);

  // compose complete message
  sprintf(str, "%s%d\n%s%s\n%s%s\n", ID_PREFIX, player_number, LEFT_ADDRESS_PREFIX, left_address_str, RIGHT_ADDRESS_PREFIX, right_address_str);

  send_to(player_address, str);
}

int main (int argc, char *argv[]) {
  in_port_t listen_port;

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

  players = malloc(sizeof(struct addrinfo*) * num_players);

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

  int first_player = randr(0, num_players-1, 42);

  // REQUIRED OUTPUT
  printf("All players present, sending potato to player %d\n", first_player);


  free(players);
  exit(0);
}
