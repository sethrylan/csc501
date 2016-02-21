/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *  Author...........: Vincent W. Freeh
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // _POSIX_HOST_NAME_MAX, PATH_MAX
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "parse.h"
#include "ush.h"

char *hostname, *home_directory;

extern char **environ;

// notes on path_resolution: http://man7.org/linux/man-pages/man7/path_resolution.7.html

int _where(const Cmd command) {
  // TODO: check for arg
  if (command->nargs == 2) {
    char *name = strdup(command->args[1]);
    print_list(search_path(name));
    return(EXIT_SUCCESS);
  } else {
    return(EXIT_FAILURE);
  }
}

int _unsetenv(Cmd command) {
  if (command->nargs == 2) {
    if (unsetenv(command->args[1]) != 0) {
      return(EXIT_FAILURE);
    }
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
    printf("error: could not change to directory %s \n", path);
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

//
// A simple command is a sequence of words, the first of which specifies the command to be executed.
//
static void evaluate_command(Cmd c) {
  int i;

  if ( c ) {
    DEBUG_PRINT("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin ){
      printf("<(%s) ", c->infile);
    }
    if ( c->out != Tnil ) {
      switch ( c->out ) {
        case Tout:
          printf(">(%s) ", c->outfile);
          break;
        case Tapp:
          printf(">>(%s) ", c->outfile);
          break;
        case ToutErr:
          printf(">&(%s) ", c->outfile);
          break;
        case TappErr:
          printf(">>&(%s) ", c->outfile);
          break;
        case Tpipe:
          printf("| ");
          break;
        case TpipeErr:
          printf("|& ");
          break;
        default:
          fprintf(stderr, "Shouldn't get here\n");
          exit(EXIT_FAILURE);
      }
    }

    if ( c->nargs > 1 ) {
      DEBUG_PRINT("[");
      for ( i = 1; c->args[i] != NULL; i++ ){
        DEBUG_PRINT("%d:%s,", i, c->args[i]);
      }
      DEBUG_PRINT("\b]");
    }
    DEBUG_PRINT("\n");

    // this driver understands one command
    if (matches(c->args[0], "end") || matches(c->args[0], "logout")) {
      _logout();
      return;
    }
    if (matches(c->args[0], "pwd")) {
      _pwd();
      return;
    }
    if (matches(c->args[0], "cd")) {
      _cd(c->args[1]);
      return;
    }
    if (matches(c->args[0], "echo")) {
      _echo(c);
      return;
    }
    if (matches(c->args[0], "setenv")) {
      _setenv(c);
      return;
    }
    if (matches(c->args[0], "unsetenv")) {
      _unsetenv(c);
      return;
    }
    if (matches(c->args[0], "where")) {
      _where(c);
      return;
    }

    execute(c);
    return;
  }
}

//
// A pipeline is a sequence of one or more simple commands separated by | or |&.
//
static void evaluate_pipe(Pipe p) {
  Cmd c;

  if ( p == NULL ){
    return;
  }

  DEBUG_PRINT("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  int i = 0;
  for ( c = p->head; c != NULL; c = c->next ) {
    i++;
    DEBUG_PRINT("  Cmd #%d: ", i);
    evaluate_command(c);
  }
  DEBUG_PRINT("End pipe\n");
  evaluate_pipe(p->next);
}

int main() {
  Pipe p;
  // char buff[PATH_MAX + 1];
  home_directory = getcwd(NULL, PATH_MAX + 1 );
  hostname = malloc(_POSIX_HOST_NAME_MAX);

  // replace with $HOSTNAME if set in environment
  if (getenv("HOSTNAME")) {
    hostname = getenv("HOSTNAME");
  } else {
    if (gethostname(hostname, _POSIX_HOST_NAME_MAX) != 0) {
      die("could not get hostname\n");
    }
  }

  while ( 1 ) {
    printf("%s%% ", hostname);
    p = parse();
    evaluate_pipe(p);
    freePipe(p);
  }
}

/*........................ end of main.c ....................................*/
