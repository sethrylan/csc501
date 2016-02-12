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

void search_path(const char *file) {
  char buf[PATH_MAX];
  if (*file == '\0') {
    return;
  }

  // if (strchr (file, '/') != NULL) {
  int got_eacces;
  size_t len, pathlen;
  char *name, *p;
  char *path = getenv("PATH");
  if (path == NULL)
    path = ":/bin:/usr/bin";

  len = strlen(file) + 1;
  pathlen = strlen(path);
  /* Copy the file name at the top.  */
  name = memcpy(buf + pathlen + 1, file, len);
  /* And add the slash.  */
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
      /* Two adjacent colons, or a colon at the beginning or the end
         of `PATH' means to search the current directory.  */
      startp = name + 1;
    } else {
      startp = memcpy(name - (p - path), path, p - path);
    }

    if (access(startp, X_OK) == 0) {
      printf("%s\n", startp);
    }
  } while (*p++ != '\0');
}

int _where(const Cmd command) {
  // TODO: check for arg
  char *name = strdup(command->args[1]);
  search_path(name);
  return(EXIT_SUCCESS);
}

int _unsetenv(Cmd command) {
  if (command->nargs == 2) {
    if (unsetenv(command->args[1]) != 0) {
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
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
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
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
  free(hostname);
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
    putchar('\n');

    // this driver understands one command
    if (matches(c->args[0], "end") || matches(c->args[0], "logout")) {
      _logout();
    }
    if (matches(c->args[0], "pwd")) {
      _pwd();
    }
    if (matches(c->args[0], "cd")) {
      _cd(c->args[1]);
    }
    if (matches(c->args[0], "echo")) {
      _echo(c);
    }
    if (matches(c->args[0], "setenv")) {
      _setenv(c);
    }
    if (matches(c->args[0], "unsetenv")) {
      _unsetenv(c);
    }
    if (matches(c->args[0], "where")) {
      _where(c);
    }



  }
}

//
// A pipeline is a sequence of one or more simple commands separated by | or |&.
//
static void evaluate_pipe(Pipe p) {
  int i = 0;
  Cmd c;

  if ( p == NULL ){
    return;
  }

  DEBUG_PRINT("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  for ( c = p->head; c != NULL; c = c->next ) {
    DEBUG_PRINT("  Cmd #%d: ", ++i);
    evaluate_command(c);
  }
  DEBUG_PRINT("End pipe\n");
  evaluate_pipe(p->next);
}

int main(int argc, char *argv[]) {
  Pipe p;
  // char buff[PATH_MAX + 1];
  home_directory = getcwd(NULL, PATH_MAX + 1 );
  hostname = malloc(_POSIX_HOST_NAME_MAX);
  int got_host = gethostname(hostname, _POSIX_HOST_NAME_MAX);
  if (got_host != 0) {
    die("could not get hostname\n");
  }

  while ( 1 ) {
    DEBUG_PRINT("%s%% ", hostname);
    p = parse();
    evaluate_pipe(p);
    freePipe(p);
  }
}

/*........................ end of main.c ....................................*/
