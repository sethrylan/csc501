#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // _POSIX_HOST_NAME_MAX, PATH_MAX
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/resource.h>
#include "parse.h"
#include "ush.h"

// variables defined in main.c, but declared also in ush.c
char *hostname, *home_directory;
int pipefd[2][2];
int pipe_index = 0;

static int evaluate_command(Cmd c);


Cmd make_command_from_args(Cmd c) {
  Cmd cTemp;
  cTemp = malloc(sizeof(Cmd));
  cTemp->in = c->in;
  cTemp->out = c->out;
  cTemp->exec = c->exec;
  cTemp->next = c->next;
  cTemp->maxargs = c->maxargs;
  cTemp->nargs = c->nargs - 2;
  cTemp->args = malloc(c->maxargs * sizeof(char*));
  int i;
  for (i = 0; i < cTemp->nargs; i++) {
    DEBUG_PRINT("copying arg: %s\n", c->args[i+2]);
    cTemp->args[i] = malloc(strlen(c->args[i+2]));
    strcpy(cTemp->args[i], c->args[i+2]);
  }
  return cTemp;
}

int _nice(Cmd c) {
  int priority = 4;
  if(c->args[1] != NULL) {
    priority = atoi(c->args[1]);
    if(priority < -20) {
      priority = -20;
    } else if(priority > 20) {
      priority = 20;
    }
  }

  DEBUG_PRINT("priority = %d\n", priority);
  setpriority(PRIO_PROCESS, 0, priority);

  if(c->args[2] != NULL) {
    Cmd cNew = make_command_from_args(c);
    evaluate_command(cNew);
    free(cNew->args);
    free(cNew);
  }

  return EXIT_SUCCESS;
}

void setup_pipe_input (int index, Cmd c) {
  int is_followed = is_pipe(c->out);
  if (is_pipe(c->in)) {
    if (is_followed) {
      // preempt pipe_index, which was toggled for next command
      DEBUG_PRINT("child(%s): dup2[index=%d -> STDIN_FILENO]\n", c->args[0], !index);
      dup2(pipefd[!index][STDIN_FILENO], STDIN_FILENO);
    } else {
      DEBUG_PRINT("child(%s): dup2[index=%d -> STDIN_FILENO]\n", c->args[0], index);
      dup2(pipefd[index][STDIN_FILENO], STDIN_FILENO);
    }
  }
}

void setup_pipe_output (int index, Cmd c) {
  if (is_pipe(c->out)) {
    DEBUG_PRINT("child(%s): dup2[STDOUT_FILENO -> index=%d]\n", c->args[0], index);
    dup2(pipefd[index][STDOUT_FILENO], STDOUT_FILENO);
    if (c->out == TpipeErr) {
      dup2(pipefd[index][STDOUT_FILENO], STDERR_FILENO);
    }
  }
}

void make_pipe (int index) {
  if (pipe(pipefd[index]) < 0) {
    die("Pipe creation failed\n");
  }
  // pipe_fds[pipe_index][1] is now writable
  // pipe_fds[pipe_index][0] is now readable
}

void setup_file_output (Cmd c) {
  // > and >> redirection; open() file and set to filedescriptor array index 1 (stdout)
  // >& and >>& redirection; same, but for stderr (index 2)
  if (is_file(c->out)) {
    // open outfile; create it doesn't exist, truncate or append depending on out token
    // create file in mode 644, -rw-r--r--
    int append = (c->out == Tapp || c->out == TappErr);
    int out = open(c->outfile, (append ? O_APPEND : O_TRUNC) | O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);

    if (c->out == Tout || c->out == Tapp) {
      dup2(out, STDOUT_FILENO);               // set c->outfile to stdout
    }
    if (c->out == ToutErr || c->out == TappErr) {
      // in bash, equivalent to any of the forms of input redirection
      //    program &>word
      //    program >&word
      //    program >word 2>&1
      dup2(out, STDOUT_FILENO);
      dup2(out, STDERR_FILENO);               // redirect stderr (2) to output file
    }
    close(out);
  }
}

void setup_file_input (Cmd c) {
  // < redirection; open() file and set to filedescriptor array index 0 (stdin)
  if (c->in == Tin) {
    clearerr(stdin);
    int inFile = open(c->infile, O_RDONLY);
    dup2(inFile, STDIN_FILENO);
  }
}

void close_pipes (int index, Cmd c) {
  if (is_pipe(c->out)) {
    DEBUG_PRINT("parent(%s): close[index=%d][STDOUT_FILENO]\n", c->args[0], index);
    close(pipefd[index][STDOUT_FILENO]);
  }

  if (is_pipe(c->in)) {
    if (is_pipe(c->out)) {
      // preempt pipe_index, which was toggled for next command
      DEBUG_PRINT("parent(%s): close[index=%d][STDIN_FILENO]\n", c->args[0], !index);
      close(pipefd[!index][STDIN_FILENO]);
    } else {
      DEBUG_PRINT("parent(%s): close[index=%d][STDIN_FILENO]\n", c->args[0], index);
      close(pipefd[index][STDIN_FILENO]);
    }
  }
}

//
// A simple command is a sequence of words, the first of which specifies the command to be executed.
//
static int evaluate_command(Cmd c) {
  pid_t pid;                           // child process pid
  int status;

  // DEBUG_PRINT("nargs = %d\n", c->nargs);
  if (matches(c->args[0], "nice")) {
    return _nice(c);
  }

  print_command_info(c);

  fflush(NULL);                        // flush all streams

  if (!strcmp(c->args[0], "end")) {    // exit at EOF
    printf("\n");
    exit(0);
  }

  // execute special case builtins (no subshell)
  if (!is_subshell_builtin(c)) {
    return builtin(c);
  }

  if (is_pipe(c->out)) {
    pipe_index = !pipe_index;
    DEBUG_PRINT("before fork(%s): make_pipe[%d]\n", c->args[0], pipe_index);
    make_pipe(pipe_index);
  }

  if ((pid = fork()) < 0) {         // fork child process
    die("fork() for child process failed\n");
  } else if (pid == 0) {            // fork() returns a value of 0 to the child process
    DEBUG_PRINT("child(%s): pid=%d\n", c->args[0], pid);

    setup_pipe_input(pipe_index, c);
    setup_pipe_output(pipe_index, c);
    setup_file_output(c);
    setup_file_input(c);

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
    DEBUG_PRINT("parent(%s): wait for %d\n", c->args[0], pid);
    waitpid(pid, &status, 0);       // wait/join for child process
    close_pipes(pipe_index, c);
    return status;
  }
  return 0;
}

//
// A pipeline is a sequence of one or more simple commands separated by | or |&.
//
static void evaluate_pipe(Pipe command_line_pipe) {
  DEBUG_PRINT("start evaluate_pipe\n");
  Cmd c;
  pipe_index = 0;

  if ( command_line_pipe == NULL ){
    return;
  }

  // save_std_streams();

  DEBUG_PRINT("Begin pipe%s\n", command_line_pipe->type == Pout ? "" : " Error");
  int i = 0;
  for ( c = command_line_pipe->head; c != NULL; c = c->next ) {
    i++;
    DEBUG_PRINT("  Cmd #%d: ", i);
    evaluate_command(c);
  }
  DEBUG_PRINT("End pipe\n");
  // restore_std_streams();
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
    fflush(NULL);     // flush all streams
  }
}


int main() {
  Pipe command_line_pipe;
  // char buff[PATH_MAX + 1];
  home_directory = getcwd(NULL, PATH_MAX + 1 );
  hostname = malloc(_POSIX_HOST_NAME_MAX);

  ignore_signals();

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
