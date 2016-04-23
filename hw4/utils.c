#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

int count_occurences(const char c, const char *s) {
  int i;
  for (i=0; s[i]; s[i]==c ? i++ : *s++);
  return i;
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

int ends_with(const char *string, const char *compare) {
  return !strncmp(string + (strlen(string)-strlen(compare)), compare, strlen(compare));
}

void free_char_list(char **list, int length) {
  for (int i = 0; i <= length; i++) {
    free(list[i]);
  }
  free(list);
}
