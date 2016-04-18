#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fuse.h>
#include <errno.h>
// #include <strings.h>
// #include <unistd.h>
// #include <signal.h>
#include "ramdisk.h"
#include "utils.h"

char *mount_path;
long max_bytes;
long current_bytes;

rd_file *root;

// http://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/callbacks.html

int get_fd (char *path, rd_file_type file_type) {
  return -1;
}

int rd_opendir (const char * path, struct fuse_file_info *fi) {
  rd_file * file;
  int ret_val = 0;
  DEBUG_PRINT("r_opendir called, path:%s, fi:%ld\n", path, fi);

  if (!path){
    return -ENOENT;
  }

  if (matches(path, "/")){
    return ret_val;
  }

  if (ends_with(path, "/")) {
    DEBUG_PRINT(ramdisk_engine->lgr, "r_open, path ended with /\n", LEVEL_ERROR);
    return -ENOENT;
  }

  file = get_fd(path, DIRECTORY);

  if(!file) {
    ret_val = -ENOENT;
  }

  return ret_val;
}

static int rd_open(const char *path, struct fuse_file_info *fi){
  rd_file *file;
  int ret_val = 0;

  DEBUG_PRINT("f_open called, path:%s, fi:%ld, O_RDONLY:%d, O_WRONLY:%d, O_RDWR:%d, O_APPEND:%d, O_TRUNC:%d\n", path, file_info, file_info->flags&O_RDONLY, file_info->flags&O_WRONLY, file_info->flags&O_RDWR, file_info->flags&O_APPEND, file_info->flags&O_TRUNC);

  if (path == NULL || matches(path, "/") || ends_with(path, "/")) {
    DEBUG_PRINT("path is NULL or a directory\n");
    return -ENOENT;
  }

  file = get_fd(path, REGULAR);

  if (!file) {
    ret_val = -ENOENT;
  }

  return ret_val;
}

static struct fuse_operations operations = {
  .open      = rd_open
  // .create    = rd_create,
  // .read      = rd_read,
  // .write     = rd_write,    //==> ssize_t write(int filedes, const void * buf , size_t nbytes ) in POSIX
  // .unlink    = rd_unlink,
  // .flush     = rd_flush,    //==> close() in POSIX
  // .opendir   = rd_opendir,
  // .readdir   = rd_readdir,
  // .mkdir     = rd_mkdir,
  // .rmdir     = rd_rmdir,
  // .getattr   = rd_getattr_wrapper,
  // .fgetattr  = rd_fgetattr_wrapper, //==> int fstat(int pathname , struct stat * buf ) in POSIX
  // .truncate  = rd_truncate_wrapper,
  // .ftruncate = rd_ftruncate_wrapper,

  // .rename   = rd_rename,
  // .access   = rd_access,
  // .chmod    = rd_chmod,
  // .chown    = rd_chown,
  // .utimens  = rd_utimens,
};

int main (int argc, char *argv[]) {

  // char *fuse_argv[2];

  // read parameters from command line
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <directory-path> <size-MB>\n", argv[0]);
    exit(1);
  }

  mount_path = argv[1];
  max_bytes = atoi(argv[2]) * 1024 * 1024;

  if (max_bytes < 0) {
    printf("size-MB cannot be less than 0\n");
    exit(1);
  }

  return fuse_main(argc - 1, argv, &operations, NULL);
}


