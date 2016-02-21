#include "list.h"

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

void  execute (char **argv) {
  pid_t pid;          // child process pid
  int status;

  if ((pid = fork()) < 0) {     // fork child process
    die("fork() for child process failed\n");
  } else if (pid == 0) {
    if (execvp(*argv, argv) < 0) {     /* execute the command  */
      die("exec failed\n");
    }
  } else {
    while (wait(&status) != pid) // wait/join for child process
    ;
  }
}

// Returns an array of executable paths that match the passed filename
// see implementation from execvp.c
node* search_path (const char *file) {
  node *list = NULL;
  char buf[PATH_MAX];
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
