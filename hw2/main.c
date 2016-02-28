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

// variables defined in main.c, but declared also in ush.c
char *hostname, *home_directory;

//
// A simple command is a sequence of words, the first of which specifies the command to be executed.
//
static int evaluate_command(Cmd c) {
  pid_t pid;          // child process pid
  int status;

  print_command_info(c);

  fflush(stdout);

  // exit at EOF
  if (!strcmp(c->args[0], "end")) {
    printf("\n");
    exit(0);
  }

  // execute special case builtins (no subshell)
  if (matches(c->args[0], "logout") || matches(c->args[0], "cd") ) {
    return builtin(c);
  }

  if ((pid = fork()) < 0) {         // fork child process
    die("fork() for child process failed\n");
  } else if (pid == 0) {            // fork() returns a value of 0 to the child process

    // > and >> redirection; open() file and set to filedescriptor array index 1 (stdout)
    // >& and >>& redirection; same, but for stderr (index 2)
    if (is_file(c->out)) {

      // open outfile; create it doesn't exist, truncate or append depending on out token
      // create file in mode 644, -rw-r--r--
      int append = (c->out == Tapp || c->out == TappErr);
      int out = open(c->outfile, (append ? O_APPEND : O_TRUNC) | O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      // set c->outfile to stdout

      fflush(stdout);               // flush anything buffered in stdout so that it doesn't go to output file

      if (c->out == Tout || c->out == Tapp) {
        dup2(out, STDOUT_FILENO);
      }
      if (c->out == ToutErr || c->out == TappErr) {
        // in bash, equivalent to any of the forms of input redirection
        //    program &>word
        //    program >&word
        //    program >word 2>&1
        dup2(out, STDOUT_FILENO);
        dup2(out, STDERR_FILENO);    // redirect stderr (2) to output file
      }
      close(out);
    }

    // < redirection; open() file and set to filedescriptor array index 0 (stdin)
    if ( c->in == Tin ){
      clearerr(stdin);
      int inFile = open(c->infile, O_RDONLY);
      dup2(inFile, STDIN_FILENO);
    }

    int retval = builtin(c);
    if (retval != 127) {
      exit(retval);
    }

    node *exec_list = search_path(c->args[0]);
    if (exec_list == NULL) {
      printf("-ush: %s: command not found\n", c->args[0]);
      exit(127);
    }

    // todo: replace with execv()
    if (execvp(exec_list->value, c->args) < 0) {   // execute the command; doesn't return unless there is an error
      fprintf(stderr, "exec failed\n");
      exit(126);
    }
  } else {                          // fork() returns the process ID of the child process to the parent process
    waitpid(pid, &status, 0);       // wait/join for child process
    return status;
  }
  return 0;
}

//
// A pipeline is a sequence of one or more simple commands separated by | or |&.
//
static void evaluate_pipe(Pipe command_line_pipe) {
  Cmd c;
  int pipe_index = 0;

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
