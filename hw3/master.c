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
  char buffer[MAX_RECV_SIZE];
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

  read_message(accept_fd, buffer, MAX_RECV_SIZE);

  char *token = strtok(buffer, "\n");
  while (token) {
    DEBUG_PRINT("accept_checkin(): %s\n", token);
    if (begins_with(token, CONNECT_PREFIX)) {
      char *port_string = malloc(10);
      strncpy(port_string, token + strlen(CONNECT_PREFIX), strlen(token) - strlen(CONNECT_PREFIX));
      DEBUG_PRINT("accept_checkin(): Adding player #%d as %s:%s\n", players_connected, host, port_string);
      struct addrinfo *player_listener = gethostaddrinfo(host, atoi(port_string));
      players[players_connected] = player_listener;
    }
    token = strtok(NULL, "\n");
  }
  DEBUG_PRINT("accept_checkin(): checkins finished\n");
  players_connected++;
}

void recv_messages(int listen_socket_fd) {
  char buffer[MAX_RECV_SIZE];
  struct sockaddr_in incoming;
  socklen_t len = sizeof(incoming);

  while (1) {
    int accept_fd = accept(listen_socket_fd, (struct sockaddr *)&incoming, &len);        // block until a client connects to the server, then return new file descriptor
    if ( accept_fd < 0 ) {
      perror("bind");
      exit(accept_fd);
    }

    read_message(accept_fd, buffer, MAX_RECV_SIZE);

    char *token = strtok(buffer, "\n");
    while (token) {
      DEBUG_PRINT("recv_player_info(): %s\n", token);
      if (begins_with(token, ROUTE_PREFIX)) {

        printf("Trace of potato:\n");
        // TODO: print trace
        return;
      }
      token = strtok(NULL, "\n");
    }
    // TODO: free(token);
  }
}


void send_info_to_player(int player_number) {
  char host[HOSTNAME_LENGTH], service[20], str[250], left_address_str[100], right_address_str[100];
  struct addrinfo *player_address = players[player_number];
  int left_player_number  = (player_number - 1) % num_players;
  int right_player_number = (player_number + 1) % num_players;
  struct addrinfo *left_address   = players[left_player_number];
  struct addrinfo *right_address  = players[right_player_number];

  DEBUG_PRINT("send_info_to_player(%d)\n", player_number);

  // compose left_address_str for left neighbor
  getnameinfo(left_address->ai_addr, left_address->ai_addrlen, host, sizeof host, service, sizeof service, 0);
  sprintf(left_address_str, "%d:%s:%s", left_player_number, host, service);

  // compose right_address_str for right neighbor
  getnameinfo(right_address->ai_addr, right_address->ai_addrlen, host, sizeof host, service, sizeof service, 0);
  sprintf(right_address_str, "%d:%s:%s", right_player_number, host, service);

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

  if (num_players < 1 || num_players > MAX_PLAYERS || hops < 0 || hops > MAX_HOPS) {
    fprintf(stderr, "Usage: %s <number-of-players> <hops>\n", argv[0]);
    fprintf(stderr, "‹number-of-players› must be > 0 and ≤ %d and ‹hops› must be ≥ 0 and ≤ %d\n", MAX_PLAYERS, MAX_HOPS);
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

  char str[strlen(ROUTE_PREFIX) + MAX_HOPS_STRLEN + 10];
  sprintf(str, "%s%0*d:\n", ROUTE_PREFIX, MAX_HOPS_STRLEN, hops);  // todo: MAX_HOPS_STRLEN
  send_to(players[first_player], str);

  recv_messages(listen_socket);

  for (i = 0; i < num_players; i++) {
    send_to(players[i], CLOSE);
  }
  close(listen_socket);
  free(players);
  exit(0);
}
