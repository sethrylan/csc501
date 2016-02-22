#include "list.h"

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif


int builtin(Cmd c);
int _where(const Cmd command);
int _unsetenv(Cmd command);
int _setenv(Cmd command);
int _echo(Cmd command);
int _cd(char *path);
int _pwd();
void _logout();

void die (const char *msg);
int matches (const char *string, const char *compare);

int execute (Cmd c);

node* search_path (const char *filename);

