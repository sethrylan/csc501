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

void  execute(char **argv) {
  pid_t pid;          // child process pid
  int status;

  if ((pid = fork()) < 0) {     // fork child process
    die("*** forking child process failed\n");
  } else if (pid == 0) {
  if (execvp(*argv, argv) < 0) {     /* execute the command  */
  printf("*** ERROR: exec failed\n");
  exit(1);
  }
  }
  else {                                  /* for the parent:      */
  while (wait(&status) != pid)       /* wait for completion  */
  ;
  }
}

void search_path(const char *file) {
  char buf[PATH_MAX];
  if (*file == '\0') {
    return;
  }

  // if (strchr (file, '/') != NULL) {
  int got_eacces;
  size_t len, pathlen;
  char *name, *p;
  char *path = getenv("PATH");
  if (path == NULL)
    path = ":/bin:/usr/bin";

  len = strlen(file) + 1;
  pathlen = strlen(path);
  /* Copy the file name at the top.  */
  name = memcpy(buf + pathlen + 1, file, len);
  /* And add the slash.  */
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
      /* Two adjacent colons, or a colon at the beginning or the end
         of `PATH' means to search the current directory.  */
      startp = name + 1;
    } else {
      startp = memcpy(name - (p - path), path, p - path);
    }

    if (access(startp, X_OK) == 0) {
      printf("%s\n", startp);
    }
  } while (*p++ != '\0');
}
