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
int listen_port;

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
  sprintf(str, "CONNECT:%d\n", listen_port);
  send_to(address, str);
}

void recv_player_info(int socket_fd) {

  char buffer[512];
  struct sockaddr_in incoming;

  socklen_t len = sizeof(incoming);
  int accept_fd = accept(listen_socket, (struct sockaddr *)&incoming, &len);        // block until a client connects to the server, then return new file descriptor
  if ( accept_fd < 0 ) {
    perror("bind");
    exit(accept_fd);
  }

  read_message(accept_fd, buffer, 512);

  char *token = strtok(buffer, "\n");
  while (token) {
    DEBUG_PRINT("%s\n", token);
    if (begins_with(token, "YOUARE:")) {
      char *player_number = malloc(10);
      strncpy(player_number, token + strlen("YOUARE:"), strlen(token) - strlen("YOUARE:"));
      DEBUG_PRINT("player_number = %s\n", player_number);
    }
    token = strtok(NULL, "\n");
  }
  DEBUG_PRINT(">> recv_player_info finished\n");


  // REQUIRED OUTPUT
  // TODO: get real number
  printf("Connected as player %d\n", 1);
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
  // read_and_send(s);

  close_player(listen_socket);
  return 0;    // never reachs here
}
