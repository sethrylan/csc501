#include <sys/fcntl.h>
#include <sys/wait.h>
// #include <errno.h>
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

int pipefd[2];

// static char *builtins[] = { "cd", "echo", "logout", "nice", "pwd", "setenv", "unsetenv", "where" };

#define is_file(t) ((t)==Tout||(t)==Tapp||(t)==ToutErr||(t)==TappErr)
#define is_pipe(t) (((t)==Tpipe||(t)==TpipeErr))

void die (const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void setup_signals () {
  signal(SIGTSTP,SIG_IGN);  //CTRL+Z
  signal(SIGQUIT,SIG_IGN);  //CTRL+\/
  // signal(SIGINT,handle_sigint); //CTRL+C
}

void save_std_streams () {
  stdin_orig = dup(STDIN_FILENO);
  stdout_orig = dup(STDOUT_FILENO);
  stderr_orig = dup(STDERR_FILENO);

  if (stdin_orig < 0 || stdout_orig < 0 || stderr_orig < 0) {
    die("could not save streams");
  }
}

void restore_std_streams () {
  DEBUG_PRINT("restore_std_streams\n");
  if(dup2(stdin_orig, STDIN_FILENO) < 0) {
    die("restore stdin failed\n");
  }
  close(stdin_orig);
  if(dup2(stdout_orig, STDOUT_FILENO) < 0) {
    die("restore stdout failed\n");
  }
  close(stdout_orig);
  if(dup2(stderr_orig, STDERR_FILENO) < 0) {
    die("restore stderr failed\n");
  }
  close(stderr_orig);
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
      // printf("Found %s\n", startp);
    }
  } while (*p++ != '\0');
  return list;
}


int execute (Cmd c) {

  // save_std_streams();

  int inFile;

  // execute special case builtins (no fork)
  if (matches(c->args[0], "end") || matches(c->args[0], "logout") || matches(c->args[0], "cd") ) {
    return builtin(c);
  }

  pid_t pid;          // child process pid
  int status;

  if ((pid = fork()) < 0) {         // fork child process
    fprintf(stderr, "fork() for child process failed\n");
    return 1;
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
      dup2(inFile, 0);
    }

    int retval = builtin(c);
    if (retval != 127) {
      exit(retval);
    }

    node *exec_list = search_path(c->args[0]);
    if (exec_list == NULL) {
      printf("-ush: %s: command not found\n", c->args[0]);
      return 127;
    }

    // todo: replace with execv()
    if (execvp(exec_list->value, c->args) < 0) {   // execute the command; doesn't return unless there is an error
      fprintf(stderr, "exec failed\n");
      return 126;
    }
  } else {                          // fork() returns the process ID of the child process to the parent process
    waitpid(pid, &status, 0);       // wait/join for child process
    return status;
  }
  return 0;
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
  if (matches(c->args[0], "end") || matches(c->args[0], "logout")) {
    _logout();
    return 0;
  }
  if (matches(c->args[0], "pwd")) {
    _pwd();
    return 0;
  }
  if (matches(c->args[0], "cd")) {
    _cd(c->args[1]);
    return 0;
  }
  if (matches(c->args[0], "echo")) {
    _echo(c);
    return 0;
  }
  if (matches(c->args[0], "setenv")) {
    _setenv(c);
    return 0;
  }
  if (matches(c->args[0], "unsetenv")) {
    _unsetenv(c);
    return 0;
  }
  if (matches(c->args[0], "where")) {
    _where(c);
    return 0;
  }
  return 127;
}
