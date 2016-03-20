#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>

extern int h_errno;
extern const char *__progname;

void die (const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int contains (char **list, char *string, size_t length) {
  size_t i = 0;
  for( i = 0; i < length; i++) {
    if(!strcmp(list[i], string)) {
      return 1;
    }
  }
  return 0;
}

int matches (const char *string, const char *compare) {
  return !strcmp(string, compare);
}

unsigned int randr(unsigned int min, unsigned int max) {
  srand(time(NULL));
  double scaled = (double)rand()/RAND_MAX;
  return (max - min +1)*scaled + min;
}

char *gethostcanonicalname() {
  struct addrinfo hints, *servinfo;
  char host[HOSTNAME_LENGTH];
  int retval;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;          // use AF_INET6 to force IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  gethostname(host, sizeof host);
  if ((retval = getaddrinfo(host, 0, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
    exit(retval);
  }

  // struct addrinfo *p;
  // for(p = servinfo; p != NULL; p = p->ai_next) {
  //   DEBUG_PRINT("p->ai_canonname = %s\n", p->ai_canonname);
  // }

  char* canonname = strdup(servinfo->ai_canonname);
  freeaddrinfo(servinfo);
  return canonname;
}

/* Open a socket for listening
 *  1. create socket
 *  2. bind it to an address/port
 *  3. listen
 *
 * Initializes listen_address variable
 * Returns file descriptor for listen socket, or -1 if not able to listen
 */
int setup_listener(int listen_port) {
  int retval;
  struct sockaddr_in address;

  /* use address family INET and STREAMing sockets (TCP) */
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket:");
    exit(socket_fd);
  }

  /* set up the address and port */
  bzero((char *) &address, sizeof(address));   // set all values in address buffer to zero
  address.sin_family = AF_INET;                       // "the correct thing to do is to use AF_INET in your struct sockaddr_in" (http://beej.us/net2/html/syscalls.html)
  address.sin_port = htons(listen_port);              // convert port to network byte order
  address.sin_addr.s_addr = htonl(INADDR_ANY);        // All IP addresses of the host. For server code, this will always be the IP address of the machine on which the server is running.
  memset(&(address.sin_zero), '\0', 8);
  // memcpy(&listen_address.sin_addr, hp->h_addr_list[0], hp->h_length);  // alternative to INADDR_ANY, which doesn't trigger firewall protection on OSX

  // bind socket s to address sin
  // if bind() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  retval = bind(socket_fd, (struct sockaddr *)&address, sizeof(address));
  if (retval < 0) {
    perror("bind:");
    exit(retval);
  }

  // if listen() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  retval = listen(socket_fd, 5);   // second argument is size of the backlog queue (number of connections that can be waiting while the process is handling a particular connection)
  if (retval < 0) {
    perror("listen:");
    exit(retval);
  }

  return socket_fd;
}

