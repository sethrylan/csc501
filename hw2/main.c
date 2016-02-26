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
  print_command_info(c);
  execute(c);
  return;
}

//
// A pipeline is a sequence of one or more simple commands separated by | or |&.
//
static void evaluate_pipe(Pipe command_line_pipe) {
  Cmd c;

  if ( command_line_pipe == NULL ){
    return;
  }

  save_std_streams();

  DEBUG_PRINT("Begin pipe%s\n", command_line_pipe->type == Pout ? "" : " Error");
  int i = 0;
  for ( c = command_line_pipe->head; c != NULL; c = c->next ) {
    i++;
    DEBUG_PRINT("  Cmd #%d: ", i);
    evaluate_command(c);
  }
  DEBUG_PRINT("End pipe\n");
  restore_std_streams();
  evaluate_pipe(command_line_pipe->next);
}

int main() {
  Pipe command_line_pipe;
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
    command_line_pipe = parse();
    evaluate_pipe(command_line_pipe);
    freePipe(command_line_pipe);
  }
}

/*........................ end of main.c ....................................*/
