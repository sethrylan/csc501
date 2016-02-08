#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

void die (const char *msg) {
  perror(msg);
  exit(1);
}

int matches (const char *string, const char *compare) {
  return !strcmp(string, compare);
}
