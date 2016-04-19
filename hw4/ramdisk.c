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
// http://gauss.ececs.uc.edu/Courses/c4029/code/fuse/notes.html
// https://lastlog.de/misc/fuse-doc/doc/html/

boolean valid_path(const char *path) {
  if (path == NULL || matches(path, "/") || ends_with(path, "/")) {
    DEBUG_PRINT("path is NULL or a directory\n");
    return FALSE;
  }
  return TRUE;
}

rd_file* get_rd_file (const char *path, rd_file_type file_type, rd_file *root) {
  DEBUG_PRINT("get_rd_file(): %s\n", token);
  // if the path is matches the filename and type, then we have the right file
  if (matches(path, root->name) && root->type == file_type) {
    return root;
  }
  char* path_c = strdup(path);
  char *token = strtok(path_c, "/");
  free(path_c);
  if (token) {                                                      // for each part of the path
    node *current = root->files;                                    // look through the list of files/directories
    while (current) {
      rd_file *file = (rd_file*)current->file;
      // printf("%s\n", file->name);
      if (matches(token, file->name)) {                             // if one of the files matches the path piece
        int lookahead = strlen(token);
        free(token);
        return get_rd_file(path+lookahead, file_type, file);      // then try it next;
      }
      current = current->next;                                      // otherwise, keep looking through the list.
    }
  }
  free(token);
  return NULL;
}

int rd_opendir (const char * path, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_opendir called, path:%s\n", path);
  rd_file *file;

  if (!path) {
    return -ENOENT;
  }

  if (matches(path, "/")){
    return 0;
  }

  if (ends_with(path, "/")) {
    DEBUG_PRINT("r_open, path ended with /\n");
    return -ENOENT;
  }

  file = get_rd_file(path, DIRECTORY, root);

  if (!file) {
    return -ENOENT;
  }

  return 0;
}

static int rd_open(const char *path, struct fuse_file_info *fi){
  DEBUG_PRINT("rd_open called, path:%s, O_RDONLY:%d, O_WRONLY:%d, O_RDWR:%d, O_APPEND:%d, O_TRUNC:%d\n",
                              path, fi->flags&O_RDONLY, fi->flags&O_WRONLY, fi->flags&O_RDWR, fi->flags&O_APPEND, fi->flags&O_TRUNC);

  rd_file *file;

  if (!valid_path(path)) {
    return -ENOENT;
  }

  file = get_rd_file(path, REGULAR, root);

  if (!file) {
    return -ENOENT;
  }

  return 0;
}

int rd_flush (const char * path, struct fuse_file_info * fi) {
  DEBUG_PRINT("rd_flush called, path:%s\n", path);
  rd_file *file;
  if (!valid_path(path)) {
    return -ENOENT;
  }
  file = get_rd_file(path, REGULAR, root);
  if (!file) {
    return -ENOENT;
  }
  return 0;
}

static int rd_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  // TODO:
  return 0;
}

/*
static int rd_create (const char *path, mode_t mode, struct fuse_file_info *fi) {
  char **file_names;
  int i, count, flag;
  rd_file *file, *file_p, *file_c;
  int ret_val = 0;

  // sprintf(s, "r_create called, path:%s, fi:%ld\n", path, fi);

  if (path ==NULL || matches(path, "/") || ends_with(path, "/")) {
    // chat_log_level(ramdisk_engine->lgr, "r_create called, path is NULL, or only /, or ended with /\n", LEVEL_ERROR);
    return -EPERM;
  }

  file_names = break_paths(path, &count);
  if( file_names==NULL ){
    // chat_log_level(ramdisk_engine->lgr, "r_create called, cannot create file_names\n", LEVEL_ERROR);
    return -EPERM;
  }

  // check parent dir exist, and create file
  if (count != -1) {
    if (count != 0) { //if more than one level dir
      for (i=0; i<=count-1; i++) { //this for only checks if parent dis exist, doesn't create dir
        if (i == 0) {
          file_p = is_file_exist(file_names[i], root->files);
          if (file_p == NULL || file_p->type == REGULAR) {
            // sprintf(s, "r_create, dir:%s doesn't exist or it's REG-file, cannot create file, 1\n", file_names[i]);
            ret_val = -ENOENT;
            break;
          }
          continue;
        }
        file_c = is_file_exist(file_names[i], file_p->sub_files);
        if (file_c == NULL || file_c->type == REGULAR) {
          // sprintf(s, "r_create, dir:%s doesn't exist or it's REG-file, cannot create file, 2\n", file_names[i]);
          ret_val = -ENOENT;
          break;
        }
        file_p = file_c;
      }

      if (ret_val == 0){ // if parent-dir check pass, create file
        file = create_ramdisk_rf(file_names[count], file_p->name );
        if (file != NULL) {
          file->parent = file_p;
          en_list(create_l_elem(file), file_p->sub_files );
          // sprintf(s, "r_create, file:%s is created, and added to %s-dir\n", r_file->name, r_file_p->name);
        } else {
          ret = -EPERM;
        }
      }
    } else {
      // count==0, something like, "/t1"; just create new file under root dir.
      file = create_ramdisk_rf(file_names[count], "/");
      if (file != NULL) {
        en_list(create_l_elem(file), ramdisk_engine->root_files );
        // sprintf(s, "r_create, file:%s is created, and added to root-dir\n", r_file->name);
      } else {
        ret = -EPERM;
      }
    }
  } else {
    ret = -ENOENT;
    // chat_log_level(ramdisk_engine->lgr, "r_create, unknown mistakes, count is -1\n", LEVEL_ERROR);
  }

  // free files
  for(i=0; i<=count; i++) {
    free(file_names[i]);
  }
  free(file_names);
  return ret;
}
*/


static struct fuse_operations operations = {
  .open      = rd_open,
  .flush     = rd_flush,  // close()
  .read      = rd_read
  // .create    = rd_create,
  // .write     = rd_write,    //==> ssize_t write(int filedes, const void * buf , size_t nbytes ) in POSIX
  // .unlink    = rd_unlink,
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


