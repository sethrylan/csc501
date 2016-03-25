#include <netinet/in.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define HOSTNAME_LENGTH 64
#define MAX_RECV_SIZE 512

#define MAX_HOPS 10000
#define MAX_HOPS_STRLEN 5

#define MAX_PLAYERS 1000

#define CONNECT_PREFIX         "CONNECT:"   // format: "<prefix>:<listen_port>"
#define ID_PREFIX              "YOUARE:"    // format: "<prefix>:<player_number>"
#define LEFT_ADDRESS_PREFIX    "LADDR:"     // format: "<prefix>:<player_number>:<host>:<port>"
#define RIGHT_ADDRESS_PREFIX   "RADDR:"     // format: "<prefix>:<player_number>:<host>:<port>"
#define ROUTE_PREFIX           "ROUTE:"     // format: "<prefix>:<hops>:<player_number>,<player_number>,..."
#define CLOSE                  "CLOSE"     // format: "<prefix>:<hops>:<player_number>,<player_number>,..."


// typedef struct {
//   unsigned int hops;
//   unsigned int *route;
// } route;

void die (const char *msg);
// char* serialize_route(route *x);
int matches (const char *string, const char *compare);
int begins_with(const char *string, const char *compare);
unsigned int randr(unsigned int min, unsigned int max, int seed);
struct addrinfo *gethostaddrinfo(const char *hostname, int port);
char *gethostcanonicalname(const char *hostname, in_port_t port);
int setup_listener(in_port_t *listen_port);
void send_message(int socket_fd, char* message);
void read_message(int socket_fd, char *message, size_t buffer_size);
void send_to(struct addrinfo *host_address, char *message);
