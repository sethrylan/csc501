#include <netinet/in.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define HOSTNAME_LENGTH 64
#define MAX_RECV_SIZE 512

#define CONNECT_PREFIX "CONNECT:"
#define ID_PREFIX  "YOUARE:"
#define LEFT_HOST_PREFIX  "LHOST:"
#define LEFT_PORT_PREFIX  "LPORT:"
#define RIGHT_HOST_PREFIX  "LHOST:"
#define RIGHT_PORT_PREFIX  "LPORT:"

typedef struct {
  int player_id;
  struct addrinfo *address_info;
  // char address[INET_ADDRSTRLEN];
  // int listen_port;
} player;

typedef struct {
  int hops;
  int *identities;
} potato;

void die (const char *msg);
int matches (const char *string, const char *compare);
int begins_with(const char *string, const char *compare);
unsigned int randr(unsigned int min, unsigned int max, int seed);
struct addrinfo *gethostaddrinfo(const char *hostname, int port);
char *gethostcanonicalname(const char *hostname, int port);
int setup_listener(int *listen_port);
void send_message(int socket_fd, char* message);
void read_message(int socket_fd, char *message, size_t buffer_size);
void send_to(struct addrinfo *host_address, char *message);
