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

int s;     // socket file descriptor
int listen_socket;
int listen_port;

// Send "close" command to master and close socket
void close_player(int socket_fd) {
  int len = send(socket_fd, "close", 5, 0);
  if (len != 5) {
    perror("send");
    exit(1);
  }
  close(socket_fd);
  exit(0);
}

// SIGINT (^c) handler
void intHandler() {
  close_player(s);
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

void send_player_info(int socket_fd) {
  char str[100];
  unsigned long len;
  sprintf(str, "CONNECT:%d\n", listen_port);
  str[strlen(str)] = '\0';
  len = send(socket_fd, str, strlen(str), 0);
  DEBUG_PRINT("len = %lu\n", len);
  if (len != strlen(str)) {
    perror("send");
    exit(1);
  }
}

void recv_player_info(int socket_fd) {

}

int main (int argc, char *argv[]) {
  int retval;
  struct addrinfo *master_info;

  signal(SIGINT, intHandler);

  /* read host and port number from command line */
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <master-machine-name> <port-number>\n", argv[0]);
    exit(1);
  }

  master_info = gethostaddrinfo(argv[1], atoi(argv[2]));
  if (master_info == NULL) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], argv[1]);
    exit(1);
  }

  /* use address family INET and STREAMing sockets (TCP) */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("socket:");
    exit(s);
  }

  // connect to socket at above addr and port
  // if connect() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  retval = connect(s, master_info->ai_addr, master_info->ai_addrlen);
  if (retval < 0) {
    perror("connect");
    exit(retval);
  }

  listen_port = 0;
  listen_socket = setup_listener(&listen_port);

  // int one = 1;
  // setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

  // REQUIRED OUTPUT
  // TODO: get real number
  printf("Connected as player %d\n", 1);

  send_player_info(s);
  recv_player_info(listen_socket);
  // read_and_send(s);

  close_player(s);
  return 0;    // never reachs here
}
