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
#include "parse.h"
#include "ush.h"

char *hostname;


// notes on path_resolution: http://man7.org/linux/man-pages/man7/path_resolution.7.html

int cd(char *path) {
  if (!path) {
    path = getenv("HOME");
  }
  if (chdir(path) != 0) {
    printf("error: could not change to directory %s \n", path);
  }
  return EXIT_SUCCESS;
}

int pwd() {
  char* cwd;
  char buff[PATH_MAX + 1];
  cwd = getcwd(buff, PATH_MAX + 1 );
  if(cwd != NULL) {
    printf("%s\n", cwd);
  }
  return EXIT_SUCCESS;
}

void logout() {
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
      logout();
    }
    if (matches(c->args[0], "pwd")) {
      pwd();
    }
    if (matches(c->args[0], "cd")) {
      cd(c->args[1]);
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
