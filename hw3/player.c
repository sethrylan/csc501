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

// for set_socket
// #include <netinet/tcp.h>

#include "utils.h"

int listen_socket;
int listen_port;

// Send "close" command to master and close socket
void close_player(int socket_fd) {
  // int len = send(socket_fd, "close", 5, 0);
  // if (len != 5) {
  //   perror("send");
  //   exit(1);
  // }
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
  // REQUIRED OUTPUT
  // TODO: get real number
  printf("Connected as player %d\n", 1);

}

int main (int argc, char *argv[]) {
  int s;     // socket file descriptor
  int retval;
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

  close_player(s);
  return 0;    // never reachs here
}
