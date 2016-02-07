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
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
#include "ush.h"

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
          exit(-1);
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
    if ( !strcmp(c->args[0], "end") ){
      exit(0);
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
  char *hostname = malloc(_POSIX_HOST_NAME_MAX);
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
