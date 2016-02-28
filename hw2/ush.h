#include "list.h"

// static char *builtins[] = { "cd", "echo", "logout", "nice", "pwd", "setenv", "unsetenv", "where" };
// static int num_builtins = 8;
static char *rc_filename = "/.ushrc";

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define is_file(t) ((t)==Tout||(t)==Tapp||(t)==ToutErr||(t)==TappErr)
#define is_pipe(t) (((t)==Tpipe||(t)==TpipeErr))

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


void save_std_streams();
void restore_std_streams();

int contains(char **list, char* string, size_t length);

void print_command_info (Cmd c);
