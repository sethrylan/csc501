#include "list.h"

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

static char *rc_filename = "/.ushrc";

void die (const char *msg);

#define is_file(t) ((t)==Tout||(t)==Tapp||(t)==ToutErr||(t)==TappErr)
#define is_pipe(t) (((t)==Tpipe||(t)==TpipeErr))

// Builtins
int builtin(Cmd c);
int is_subshell_builtin(Cmd c);
int _where(const Cmd command);
int _unsetenv(Cmd command);
int _setenv(Cmd command);
int _echo(Cmd command);
int _cd(char *path);
int _pwd();
void _logout();

int matches (const char *string, const char *compare);
int contains(char **list, char* string, size_t length);

node* search_path (const char *filename);

void setup_signals();
void save_std_streams();
void restore_std_streams();

void print_command_info (Cmd c);
