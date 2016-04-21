
#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__);
#else
#define DEBUG_PRINT(...) do { } while ( 0 )
#endif

void die (const char *msg);
int mod(int a, int b);
int count_occurences(const char c, const char *string);
int matches (const char *string, const char *compare);
int begins_with(const char *string, const char *compare);
int ends_with(const char *string, const char *compare);
