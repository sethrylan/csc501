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

//
// A simple command is a sequence of words, the first of which specifies the command to be executed.
//
static void evaluate_command(Cmd c) {
  int i;

  if ( c ) {
    DEBUG_PRINT("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin ){
      DEBUG_PRINT("<(%s) ", c->infile);
    }
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
      for ( i = 1; c->args[i] != NULL; i++ ){
        DEBUG_PRINT("%d:%s,", i, c->args[i]);
      }
      DEBUG_PRINT("\b]");
    }
    DEBUG_PRINT("\n");

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
