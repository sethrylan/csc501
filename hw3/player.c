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

int listen_socket;
in_port_t listen_port;
int player_number;
struct addrinfo *left_addrinfo, *right_addrinfo;

// Send "close" command to master and close socket
void close_player(int socket_fd) {
  close(socket_fd);
  exit(0);
}

// SIGINT (^c) handler
void intHandler() {
  close_player(listen_socket);
}

// read a string from the terminal and send on socket
void read_and_send(int socket_fd) {
  char str[1000];   // roughly the size of 1 packet
  unsigned long len;
  while (fgets(str, 1000, stdin) != NULL) {
    if (str[strlen(str)-1] == '\n') {
      str[strlen(str)-1] = '\0';
    }
    len = send(socket_fd, str, strlen(str), 0);
    if (len != strlen(str)) {
      perror("send");
      exit(1);
    }
  }
}

void send_player_info(struct addrinfo *address) {
  char str[100];
  sprintf(str, "%s%d\n", CONNECT_PREFIX, listen_port);
  send_to(address, str);
}

void recv_player_info(int listen_socket_fd) {
  char buffer[512];
  struct sockaddr_in incoming;
  socklen_t len = sizeof(incoming);

  int accept_fd = accept(listen_socket_fd, (struct sockaddr *)&incoming, &len);        // block until a client connects to the server, then return new file descriptor
  if ( accept_fd < 0 ) {
    perror("bind");
    exit(accept_fd);
  }

  read_message(accept_fd, buffer, 512);

  char *token = strtok(buffer, "\n");
  while (token) {
    DEBUG_PRINT("%s\n", token);
    if (begins_with(token, ID_PREFIX)) {
      char *player_number_str = malloc(10);
      strncpy(player_number_str, token + strlen(ID_PREFIX), strlen(token) - strlen(ID_PREFIX));
      DEBUG_PRINT("player_number = %s\n", player_number_str);
      player_number = atoi(player_number_str);
    }
    token = strtok(NULL, "\n");
  }
  DEBUG_PRINT(">> recv_player_info finished\n");

  // REQUIRED OUTPUT
  printf("Connected as player %d\n", player_number);
}

int main (int argc, char *argv[]) {
  struct addrinfo *master_info;

  signal(SIGINT, intHandler);

  /* read host and port number from command line */
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <master-machine-name> <port-number>\n", argv[0]);
    exit(1);
  }

  master_info = gethostaddrinfo(argv[1], atoi(argv[2]));

  listen_port = 0;
  listen_socket = setup_listener(&listen_port);

  // int one = 1;
  // setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

  send_player_info(master_info);
  recv_player_info(listen_socket);
  // Uncomment to enable an interactive write to the socket // read_and_send(s);

  // TODO: wait for potato or close message

  close_player(listen_socket);
  return 0;    // never reachs here
}
