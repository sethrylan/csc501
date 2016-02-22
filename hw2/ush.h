#include "list.h"
#include <sys/fcntl.h>
#include <sys/wait.h> // for waitpid

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

void die (const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int matches (const char *string, const char *compare) {
  return !strcmp(string, compare);
}

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
  pid_t pid;          // child process pid
  int status;

  node *exec_list = search_path(c->args[0]);
  if (exec_list == NULL) {
    printf("-ush: %s: command not found\n", c->args[0]);
    return 127;
  }

  if ((pid = fork()) < 0) {         // fork child process
    fprintf(stderr, "fork() for child process failed\n");
    return 1;
  } else if (pid == 0) {            // fork() returns a value of 0 to the child process

    // > and >> redirection; open() file and set to filedescriptor array index 1 (stdout)
    // >& and >>& redirection; same, but for stderr (index 2)
    if (c->out != Tnil) {
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

        dup2(STDERR_FILENO, STDOUT_FILENO);    // redirect stdout (1) to stderr (2); safer than the other way, since stderr is unbuffered
        dup2(out, STDERR_FILENO);              // redirect stderr (2) to output file
      }
      close(out);

      // clearerr(stdout);
    }

    // < redirection; open() file and set to filedescriptor array index 0 (stdin)
    if ( c->in == Tin ){
      int fd = open(c->infile, O_RDONLY);
      dup2(fd, 0);
      close(fd);
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
