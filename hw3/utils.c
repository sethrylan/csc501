#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>

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

struct hostent *gethostent() {
  struct hostent *hp;
  char host[64];

  /* fill in hostent struct for self */
  gethostname(host, sizeof host);
  hp = gethostbyname(host);
  // DEBUG_PRINT("hp->h_addr_list[0] = %s\n", hp->h_addr_list[0]);
  if (hp == NULL) {
    if (h_errno == HOST_NOT_FOUND) {
      fprintf(stderr, "%s: host not found (%s)\n", __progname, host);
      exit(1);
    } else {
      perror(__progname);
      exit(1);
    }
  }
  return hp;
}
