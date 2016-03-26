#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>


extern int h_errno;
extern const char *__progname;

void die (const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int mod(int a, int b) {
  int m = a % b;
  return m < 0 ? m + b : m;
}

// see http://stackoverflow.com/questions/1745811/using-c-convert-a-dynamically-allocated-int-array-to-a-comma-separated-string-a
size_t join_integers(const unsigned int *num, size_t num_len, char *buf, size_t buf_len) {
  size_t i;
  unsigned int written = 0;
  for(i = 0; i < num_len; i++) {
    written += snprintf(buf + written, buf_len - written, (i != 0 ? ", %u" : "%u"), *(num + i));
    if(written == buf_len) {
      break;
    }
  }
  return written;
}

// reverse:  reverse string s in place
void reverse(char s[]) {
  int i, j;
  char c;
  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

// Convert integer to string.
// K&R implementation
void itoa(int n, char s[]) {
  int i, sign;
  if ((sign = n) < 0)  // record sign
    n = -n;            // make n positive
  i = 0;
  do {                 //generate digits in reverse order
    s[i++] = n % 10 + '0';   // get next digit
  } while ((n /= 10) > 0);   // delete it
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

int begins_with(const char *string, const char *compare) {
  return !strncmp(string, compare, strlen(compare));
}

// see http://c-faq.com/lib/randrange.html
unsigned int randr(unsigned int min, unsigned int max, int seed) {
  if (seed == -1) {
    srand(time(NULL));
  } else {
    srand(seed);
  }
  return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

// Beej's implementation: see http://www.beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
// get sockaddr, IPv4 or IPv6
void * get_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct addrinfo *gethostaddrinfo(const char *hostname, int port) {
  struct addrinfo hints, *server_info;
  int retval;
  char port_string[6];

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

  return server_info;
}

char *gethostcanonicalname(const char *hostname, in_port_t port) {
  struct addrinfo *server_info = gethostaddrinfo(hostname, port);
  char* canonname = strdup(server_info->ai_canonname);
  freeaddrinfo(server_info);
  return canonname;
}

// Open a socket for listening
//  1. create socket
//  2. bind it to an address/port
//  3. listen
//
// Initializes listen_address variable and sets listen_port if assigned.
// Returns file descriptor for listen socket, or -1 if not able to listen
//
int setup_listener(in_port_t *listen_port) {
  int retval;

  struct addrinfo *address = gethostaddrinfo(NULL, *listen_port);

  int socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
  if (socket_fd < 0) {
    perror("socket");
    exit(socket_fd);
  }

  const int enable_reuse = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
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
    DEBUG_PRINT("setup_listener(): port number %d\n", ntohs(sin.sin_port));
    *listen_port = ntohs(sin.sin_port);
  }

  // DEBUG_PRINT("address->ai_addr->sin_port = %d\n", ((struct sockaddr_in*)address->ai_addr)->sin_port);

  return socket_fd;
}

void send_message(int socket_fd, char* message) {
  unsigned long len;
  len = send(socket_fd, message, strlen(message), 0);
  DEBUG_PRINT("send_message(): len = %lu\n", len);
  if (len != strlen(message)) {
    perror("send");
  }
}

// Blocks until message and connection close is received, the puts message text into buffer.
// The socket_fd must be a connected socket ready for connections
// See http://www.beej.us/guide/bgnet/output/html/singlepage/bgnet.html#sendrecv
// Returns a buffer set to a null-terminated string with commands separated by \n
void read_message(int socket_fd, char *message, size_t buffer_size) {
  int m = 0;
  int num_bytes;
  char buffer[MAX_RECV_SIZE];
  bzero(message, buffer_size);

  while (1) {
    bzero(buffer, MAX_RECV_SIZE);
    if ((num_bytes = recv(socket_fd, buffer, MAX_RECV_SIZE-1, 0)) < 0) {   // block until input is read from socket
      perror("recv");
      exit(1);
    }
    DEBUG_PRINT("read_message(): num_bytes = %d\n", num_bytes);

    if (num_bytes == 0) {
      close(socket_fd);
      return;
    }

    buffer[num_bytes] = '\0';
    char *token = strtok(buffer, "\n");
    while (token) {
      if (!strcmp("close", token)) {
        close(socket_fd);
        return;
      } else {
        sprintf(message + m, "%s\n", token);
        m += strlen(token) + 1;
      }
      token = strtok(NULL, "\n");
    }
  }
}

// 0. Validate host_address
// 1. Create socket
// 2. Connect socket to host
// 3. Send message
// 4. Close socket
//
// Recommended to compose message using sprintf, which will fill remaining buffer with \0
//
void send_to(struct addrinfo *host_address, char *message) {
  if (host_address == NULL) {
    fprintf(stderr, "send_to(): host_address cannot be null\n");
    exit(1);
  }

  // use address family INET and STREAMing sockets (TCP)
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket");
    exit(socket_fd);
  }

  // connect to socket at addr and port
  // if connect() succeeds, then value of 0 is returned, otherwise -1 is returned and errno is set.
  int retval = connect(socket_fd, host_address->ai_addr, host_address->ai_addrlen);
  if (retval < 0) {
    perror("connect");
    exit(retval);
  }

  send_message(socket_fd, message);
  close(socket_fd);
}


// See http://stackoverflow.com/questions/4130147/get-local-ip-address-in-c-linux
void print_if_info() {
  struct ifaddrs *ifAddrStruct = NULL;
  struct ifaddrs *ifa = NULL;
  void *tmpAddrPtr = NULL;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
      // is a valid IP4 Address
      tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      DEBUG_PRINT("'%s': %s\n", ifa->ifa_name, addressBuffer);
     } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
      // is a valid IP6 Address
      tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
      char addressBuffer[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
      DEBUG_PRINT("'%s': %s\n", ifa->ifa_name, addressBuffer);
    }
  }
  if (ifAddrStruct!=NULL) {
    freeifaddrs(ifAddrStruct);  //remember to free ifAddrStruct
  }
  return;
}

void print_addrinfo(struct addrinfo *address) {
  struct addrinfo *a = address;
  while (a != NULL) {
    if (a->ai_addr->sa_family == AF_INET) { // check it is IP4
      // is a valid IP4 Address
      char addressBuffer[INET_ADDRSTRLEN];
      struct sockaddr_in *ip4a = (struct sockaddr_in*)a->ai_addr;
      inet_ntop(AF_INET, &(ip4a->sin_addr), addressBuffer, INET_ADDRSTRLEN);
      DEBUG_PRINT("%s IP Address %s\n", a->ai_canonname, addressBuffer);
    } else if (a->ai_addr->sa_family == AF_INET6) { // check it is IP6
      // is a valid IP6 Address
      char addressBuffer[INET6_ADDRSTRLEN];
      struct sockaddr_in6 *ip6a = (struct sockaddr_in6*)a->ai_addr;
      inet_ntop(AF_INET6, &(ip6a->sin6_addr), addressBuffer, INET6_ADDRSTRLEN);
      printf("%s IP Address %s\n", a->ai_canonname, addressBuffer);
    }
    a = a->ai_next;
  }
  return;
}

