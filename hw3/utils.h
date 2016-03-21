#include <netinet/in.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define HOSTNAME_LENGTH 64

typedef struct {
  int player_id;
  char address[INET_ADDRSTRLEN];
  int listen_port;
} player;

typedef struct {
  int hops;
  int *identities;
} potato;

void die (const char *msg);
int matches (const char *string, const char *compare);
unsigned int randr(unsigned int min, unsigned int max);
struct addrinfo *gethostaddrinfo(const char *hostname);
char *gethostcanonicalname(const char *hostname);
int setup_listener(const int listen_port);
