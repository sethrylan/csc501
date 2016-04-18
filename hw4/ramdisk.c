#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fuse.h>
// #include <strings.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <unistd.h>
// #include <signal.h>
// #include "utils.h"

char *mount_path;
int max_memory;


static struct fuse_operations operations = {
  // .open      = f_open,
  // .create    = f_create,
  // .read      = f_read,
  // .write     = f_write,    //==> ssize_t write(int filedes, const void * buf , size_t nbytes ) in POSIX
  // .unlink    = f_unlink,
  // .flush     = f_flush,    //==> close() in POSIX
  // .opendir   = f_opendir,
  // .readdir   = f_readdir,
  // .mkdir     = f_mkdir,
  // .rmdir     = f_rmdir,
  // .getattr   = f_getattr_wrapper,
  // .fgetattr  = f_fgetattr_wrapper, //==> int fstat(int pathname , struct stat * buf ) in POSIX
  // .truncate  = f_truncate_wrapper,
  // .ftruncate = f_ftruncate_wrapper,

  // .rename   = f_rename,
  // .access   = f_access,
  // .chmod    = f_chmod,
  // .chown    = f_chown,
  // .utimens  = f_utimens,
};


int main (int argc, char *argv[]) {

  // char *fuse_argv[2];

  // read parameters from command line
  if (argc < 3) {
    printf(stderr, "Usage: %s <directory-path> <size-MB>\n", argv[0]);
    exit(1);
  }

  mount_path = strndup(argv[1], PATH_MAX);
  max_memory = atoi(argv[2]) * 1024 * 1024;

  if (max_memory < 0) {
    printf("size-MB cannot be less than 0\n");
    exit(1);
  }

  return fuse_main(argc - 1, argv, &operations, NULL);
}


