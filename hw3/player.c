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
int player_number, left_player_number, right_player_number;
struct addrinfo *left_addrinfo, *right_addrinfo, *master_info;

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

      if (begins_with(token, CLOSE)) {
        close_player(listen_socket);
      }

      if (begins_with(token, ID_PREFIX)) {
        sscanf(token, "%*[^:]:%d", &player_number);
        DEBUG_PRINT("recv_player_info(): player_number = %d\n", player_number);

        // REQUIRED OUTPUT
        printf("Connected as player %d\n", player_number);
      }

      if (begins_with(token, LEFT_ADDRESS_PREFIX) || begins_with(token, RIGHT_ADDRESS_PREFIX)) {
        char host[INET6_ADDRSTRLEN];
        char prefix[strlen(LEFT_ADDRESS_PREFIX)];
        int port, neighbor_number;

        sscanf(token, "%[^:]:%d:%[^:]:%d", prefix, &neighbor_number, host, &port);  // see https://en.wikipedia.org/wiki/Scanf_format_string

        if (begins_with(LEFT_ADDRESS_PREFIX, prefix)) {
          left_addrinfo = gethostaddrinfo(host, port);
          left_player_number = neighbor_number;
        } else if (begins_with(RIGHT_ADDRESS_PREFIX, prefix)) {
          right_addrinfo = gethostaddrinfo(host, port);
          right_player_number = neighbor_number;
        }
      }

      if (begins_with(token, ROUTE_PREFIX)) {
        char hops_str[MAX_HOPS_STRLEN];
        int hops;

        strncpy(hops_str, token + strlen(ROUTE_PREFIX), MAX_HOPS_STRLEN);
        DEBUG_PRINT("recv_messages(): hops_str = %s\n", hops_str);
        hops = atoi(hops_str);

        int i = 0;
        const char *s = token + strlen(ROUTE_PREFIX) + MAX_HOPS_STRLEN + 1;
        do {
            size_t field_len = strcspn(s, ",");
            if (field_len > 0) {
              // DEBUG_PRINT("route[%d] = %.*s\n", i, (int)field_len, s);
              s += field_len;
              i++;
            }
        } while (*s++);

        char message[MAX_RECV_SIZE];
        // append this player to the routes list
        if (i == 0) {
          sprintf(message, "%s%d", token, player_number);
        } else {
          sprintf(message, "%s,%d", token, player_number);
        }

        if (i == hops - 1) {
          // REQUIRED OUTPUT
          printf("I'm it\n");
          send_to(master_info, message);
        } else {
          int left = randr(0, 1, 42);
          if (left) {
            printf("Sending potato to %d\n", left_player_number);
            send_to(left_addrinfo, message);
          } else {
            printf("Sending potato to %d\n", right_player_number);
            send_to(right_addrinfo, message);
          }
        }
      }
      token = strtok(NULL, "\n");
    }
    free(token);
  }
}

int main (int argc, char *argv[]) {
  signal(SIGINT, intHandler);

  // read host and port number from command line
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <master-machine-name> <port-number>\n", argv[0]);
    exit(1);
  }

  master_info = gethostaddrinfo(argv[1], atoi(argv[2]));

  listen_port = 0;   // pass 0 to be assigned port in ephemeral range
  listen_socket = setup_listener(&listen_port);

  // Uncomment to enable an interactive write to the socket // read_and_send(s);

  send_player_info(master_info);
  recv_messages(listen_socket);   // wait for messages (potato/route/close)
  close_player(listen_socket);
  return 0;    // never reachs here
}
