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

/* reverse:  reverse string s in place */
void reverse(char s[]) {
  int i, j;
  char c;
  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

void itoa(int n, char s[]) {
  int i, sign;
  if ((sign = n) < 0)  /* record sign */
    n = -n;          /* make n positive */
  i = 0;
  do {       /* generate digits in reverse order */
    s[i++] = n % 10 + '0';   /* get next digit */
  } while ((n /= 10) > 0);     /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
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

struct addrinfo *gethostaddrinfo(const char *hostname, int port) {
  struct addrinfo hints, *server_info;
  int retval;
  char port_string[6];

  DEBUG_PRINT("gethostaddrinfo( %s )\n", hostname);

  itoa(port, port_string);
  memset(&hints, 0, sizeof hints);

  if (hostname == NULL) {
    hints.ai_flags = AI_PASSIVE;   // roughly equivalent to address.sin_addr.s_addr = htonl(INADDR_ANY)
    // memcpy(&listen_address.sin_addr, hp->h_addr_list[0], hp->h_length);  // alternative to INADDR_ANY, which doesn't trigger firewall protection on OSX
  } else {
    hints.ai_flags = AI_CANONNAME;
  }

  hints.ai_family = AF_INET;          // use AF_INET6 to force IPv6
  hints.ai_socktype = SOCK_STREAM;

  if ((retval = getaddrinfo(hostname, port_string, &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
    exit(retval);
  }

  struct addrinfo *p;
  for(p = server_info; p != NULL; p = p->ai_next) {
    DEBUG_PRINT("p->ai_canonname = %s\n", p->ai_canonname);
  }
  return server_info;
}

char *gethostcanonicalname(const char *hostname, int port) {
  // char host[HOSTNAME_LENGTH];
  // if (hostname == NULL) {
  //   gethostname(host, sizeof host);
  // } else {
  //   strcpy(host, hostname);
  // }
  struct addrinfo *server_info = gethostaddrinfo(hostname, port);
  char* canonname = strdup(server_info->ai_canonname);
  freeaddrinfo(server_info);
  return canonname;
}

/* Open a socket for listening
 *  1. create socket
 *  2. bind it to an address/port
 *  3. listen
 *
 * Initializes listen_address variable and sets listen_port if assigned.
 * Returns file descriptor for listen socket, or -1 if not able to listen
 */
int setup_listener(int *listen_port) {
  int retval;

  struct addrinfo *address = gethostaddrinfo(NULL, *listen_port);

  int socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
  if (socket_fd < 0) {
    perror("socket");
    exit(socket_fd);
  }

  // bind socket s to address sin
  // if bind() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  retval = bind(socket_fd, address->ai_addr, address->ai_addrlen);
  if (retval < 0) {
    perror("bind");
    exit(retval);
  }

  // if listen() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  retval = listen(socket_fd, 5);   // second argument is size of the backlog queue (number of connections that can be waiting while the process is handling a particular connection)
  if (retval < 0) {
    perror("listen");
    exit(retval);
  }

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(socket_fd, (struct sockaddr *)&sin, &len) == -1){
    perror("getsockname");
  }
  else {
    DEBUG_PRINT("port number %d\n", ntohs(sin.sin_port));
    *listen_port = ntohs(sin.sin_port);
  }

  // DEBUG_PRINT("address->ai_addr->sin_port = %d\n", ((struct sockaddr_in*)address->ai_addr)->sin_port);

  return socket_fd;
}

