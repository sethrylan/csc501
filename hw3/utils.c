#include "utils.h"

// #include <sys/fcntl.h>
// #include <sys/wait.h>
// #include <signal.h>
#include <stdlib.h>
#include <stdio.h>
// #include <unistd.h>
#include <string.h>
// #include <limits.h>



void die (const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int contains(char **list, char *string, size_t length) {
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
