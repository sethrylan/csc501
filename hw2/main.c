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
#include <fcntl.h>
#include "parse.h"
#include "ush.h"

char *hostname, *home_directory;

//
// A simple command is a sequence of words, the first of which specifies the command to be executed.
//
static int evaluate_command(Cmd c) {
  int subshell_pid, status;

  print_command_info(c);

  fflush(stdout);

  // exit at EOF
  if (!strcmp(c->args[0], "end")) {
    printf("\n");
    exit(0);
  }

  if (contains(builtins, c->args[0], num_builtins)) {
    // execute special case builtins (no subshell)
    if (matches(c->args[0], "logout") || matches(c->args[0], "cd") ) {
      return builtin(c);
    } else {
      // create subshell for builtin
        // // todo: make_pipe(pipe_ref);
      if ((subshell_pid = fork()) < 0) {         // fork child process
        die("fork() for subshell process failed\n");
      } else if (subshell_pid == 0) {
        // child process
        // todo: setup_pipe_redirection(pipe_ref,STDIN_FILENO);
        DEBUG_PRINT("in subshell\n");
        return builtin(c);
      } else {                          // fork() returns the process ID of the child process to the parent process
        // todo: setup_pipe_redirection(pipe_ref,STDOUT_FILENO);
        DEBUG_PRINT("in parent shell. waiting...\n");
        waitpid(subshell_pid, &status, 0);       // wait/join for subshell
        return status;
      }
    }
  }

  execute(c);
  return 0;
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

void process_rc() {
  DEBUG_PRINT("process_rc start\n");
  int rc_file;
  int rc_stdin_orig;

  char rc_full_path[PATH_MAX];
  char *home_dir = getenv("HOME");
  strcpy(rc_full_path, home_dir);
  strcat(rc_full_path, rc_filename);

  if((rc_file = open(rc_full_path, O_RDONLY)) > 0) {
    // freopen(rc_full_path, "r", stdin);
    rc_stdin_orig = dup(STDIN_FILENO);
    DEBUG_PRINT("rc_stdin_orig = %d\n", rc_stdin_orig);
    if(rc_stdin_orig == -1) {
      die("dup failed");
    }
    if(dup2(rc_file, STDIN_FILENO)==-1) {
      die("dup failed");
    }

    close(rc_file);

    Pipe p;
    while (1) {
      p = parse();
      if (p==NULL || !strcmp(p->head->args[0], "end")){
        freePipe(p);
        break;
      }
      evaluate_pipe(p);
      freePipe(p);
    }

    DEBUG_PRINT("rc_stdin_orig = %d\n", rc_stdin_orig);
    if(dup2(rc_stdin_orig, STDIN_FILENO)==-1) {
      die("dup failed");
    }
    close(rc_stdin_orig);
    fflush(NULL);
  }
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

  process_rc();

  while ( 1 ) {
    printf("%s%% ", hostname);
    command_line_pipe = parse();
    evaluate_pipe(command_line_pipe);
    freePipe(command_line_pipe);
  }
}


/*........................ end of main.c ....................................*/
