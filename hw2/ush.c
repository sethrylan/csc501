#include <sys/fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "parse.h"
#include "ush.h"

extern char **environ;

extern char *hostname, *home_directory;

int stdout_orig, stdin_orig, stderr_orig;

int stdstream_orig[3];

void die (const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

// builtins that execute in a subsell must call exit().
int is_subshell_builtin (Cmd c) {
  int setenv_output_only = (matches(c->args[0], "setenv") && c->nargs > 1);
  return !(matches(c->args[0], "logout") || matches(c->args[0], "cd") || matches(c->args[0], "unsetenv") || setenv_output_only);
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

void handle_sigtstp(int signo) {
  DEBUG_PRINT("sending SIGTSTP to %d\n", getpid());
  kill(getpgrp(), SIGTSTP);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
}


// Ignore QUIT signals. Background jobs are immune to signals generated from the keyboard,
// including hangups (HUP). Other signals have the values that ush inherited from its environment.
// Catches the TERM signal.
void setup_signals () {
  DEBUG_PRINT("setup signals\n");
  signal(SIGTSTP, handle_sigtstp);  // CTRL+Z
  signal(SIGQUIT, SIG_IGN);  // CTRL+/
  signal(SIGINT, SIG_IGN);   //CTRL+C
}

void save_std_stream (int fd) {
  stdstream_orig[fd] = dup(fd);
  if (stdstream_orig[fd]) {
    die("could not save stream");
  }
}

void restore_std_stream (int fd) {
  if(dup2(stdstream_orig[fd], fd) < 0) {
    die("restore std stream failed");
  }
  close(stdstream_orig[fd]);
}

void save_std_streams () {
  save_std_streams(STDIN_FILENO);
  save_std_streams(STDOUT_FILENO);
  save_std_streams(STDERR_FILENO);
}

void restore_std_streams () {
  restore_std_streams(STDIN_FILENO);
  restore_std_streams(STDOUT_FILENO);
  restore_std_streams(STDERR_FILENO);
}

int matches (const char *string, const char *compare) {
  return !strcmp(string, compare);
}

// notes on path_resolution: http://man7.org/linux/man-pages/man7/path_resolution.7.html
// Returns an array of executable paths that match the passed filename
// see implementation from execvp.c
node* search_path (const char *filename) {
  node *list = NULL;
  char buf[PATH_MAX];
  char *file = strdup(filename);
  if (file == NULL || *file == '\0') {
    return NULL;
  }

  if (strchr (file, '/') != NULL) {
    return make_node(file);
  }
  int got_eacces;
  size_t len, pathlen;
  char *name, *p;
  char *path = getenv("PATH");
  if (path == NULL) {
    path = ":/bin:/usr/bin";
  }

  len = strlen(file) + 1;
  pathlen = strlen(path);
  // copy the file name at the top.
  name = memcpy(buf + pathlen + 1, file, len);
  // And add the slash.
  *--name = '/';

  got_eacces = 0;
  p = path;
  do {
    char *startp;
    path = p;
    p = strchr(path, ':');
    if (!p){
      p = strchr(path, '\0');
    }

    if (p == path) {
      // Two adjacent colons, or a colon at the beginning or the end
      // of `PATH' means to search the current directory.
      startp = name + 1;
    } else {
      startp = memcpy(name - (p - path), path, p - path);
    }

    if (access(startp, X_OK) == 0) {
      if (list == NULL) {
        list = make_node(startp);
      } else {
        push(list, startp);
      }
      DEBUG_PRINT("Found %s\n", startp);
    }
  } while (*p++ != '\0');
  return list;
}

int _where(const Cmd command) {
  if (command->nargs == 2) {
    char *name = strdup(command->args[1]);
    node *list = search_path(name);
    print_list(list);
    return(EXIT_SUCCESS);
  } else {
    return(EXIT_FAILURE);
  }
}

int _unsetenv(Cmd command) {
  if (command->nargs == 2) {
    if (unsetenv(command->args[1]) != 0) {
      fprintf(stderr, "unsetenv: could not unset variable\n");
      return(EXIT_FAILURE);
    }
  } else {
    fprintf(stderr, "unsetenv: wrong number of arguments\n");
  }
  return(EXIT_SUCCESS);
}

int _setenv(Cmd command) {
  if (command->nargs == 1) {
    int i;
    char *variable = *environ;
    for (i=1; variable; i++) {
      printf("%s\n", variable);
      variable = *(environ + i);
    }
  } else {
    if (setenv(command->args[1], command->args[2] ? command->args[2] : "", 1) != 0) {
      return(EXIT_FAILURE);
    }
  }
  return(EXIT_SUCCESS);
}

// Writes each word to the shell’s standard output,
// separated by spaces and terminated with a newline.
int _echo(Cmd command) {
  int i;
  for (i = 1; i < command->nargs; i++) {
    printf("%s ", command->args[i]);
  }
  printf("\n");
  return EXIT_SUCCESS;
}


// Change the working directory of the shell to dir, provided it is a directory and the shell has the
// appropriate permissions. Without an argument, it changes the working directory to the original
// (home) directory
int _cd(char *path) {
  if (!path) {
    path = home_directory;
  }
  if (chdir(path) != 0) {
    fprintf(stderr, "error: could not change to directory %s \n", path);
  }
  return EXIT_SUCCESS;
}

int _pwd() {
  char* cwd;
  cwd = getcwd(NULL, PATH_MAX + 1 );
  if(cwd != NULL) {
    printf("%s\n", cwd);
  }
  return EXIT_SUCCESS;
}

void _logout() {
  // free(hostname);
  printf("\n");
  exit(EXIT_SUCCESS);
}

int builtin(Cmd c) {
  // TODO: return actual values from builtins
  if (matches(c->args[0], "logout")) {
    _logout();
  }
  if (matches(c->args[0], "cd")) {
    return _cd(c->args[1]);
  }
  if (matches(c->args[0], "setenv")) {
    // special case:
    //    if no arguments, then print in subshell,
    //    else setenv/putenv in parent shell
    int retval = _setenv(c);
    if (c->nargs == 1) {
      exit(EXIT_SUCCESS);
    } else {
      return  retval;
    }
  }
  if (matches(c->args[0], "unsetenv")) {
    return(_unsetenv(c));
  }
  if (matches(c->args[0], "pwd")) {
    _pwd();
    exit(EXIT_SUCCESS);
  }
  if (matches(c->args[0], "echo")) {
    _echo(c);
    exit(EXIT_SUCCESS);
  }
  if (matches(c->args[0], "where")) {
    _where(c);
    exit(EXIT_SUCCESS);
  }
  return 127;
}


void print_command_info (Cmd c) {
  if ( c ) {
    if ( c->in != Tnil ) {
      switch ( c->in ) {
        case Tin:
          DEBUG_PRINT("<(%s) ", c->infile);
          break;
        case Tpipe:
          DEBUG_PRINT("| ");
          break;
        case TpipeErr:
          DEBUG_PRINT("|& ");
          break;
        default:
          break;
      }
    }

    DEBUG_PRINT("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);

    if ( c->out != Tnil ) {
      switch ( c->out ) {
        case Tout:
          DEBUG_PRINT(">(%s) ", c->outfile);
          break;
        case Tapp:
          DEBUG_PRINT(">>(%s) ", c->outfile);
          break;
        case ToutErr:
          DEBUG_PRINT(">&(%s) ", c->outfile);
          break;
        case TappErr:
          DEBUG_PRINT(">>&(%s) ", c->outfile);
          break;
        case Tpipe:
          DEBUG_PRINT("| ");
          break;
        case TpipeErr:
          DEBUG_PRINT("|& ");
          break;
        default:
          fprintf(stderr, "Shouldn't get here\n");
          exit(EXIT_FAILURE);
      }
    }

    if ( c->nargs > 1 ) {
      DEBUG_PRINT("[");
      int i;
      for ( i = 1; i < c->nargs; i++ ){
        DEBUG_PRINT("%d:%s,", i, c->args[i]);
      }
      DEBUG_PRINT("\b]");
    }
    DEBUG_PRINT("\n");
  }
}
