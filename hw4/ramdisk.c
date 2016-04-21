#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fuse.h>
#include <errno.h>
// #include <strings.h>
#include <unistd.h>
// #include <signal.h>
#include "ramdisk.h"
#include "utils.h"

// Global State
char *mount_path;
long max_bytes;
long current_bytes;

time_t init_time;
uid_t uid;
gid_t gid;

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

rd_file* get_file(char *name, node *list){
  node *current = list;
  while (current != NULL) {
    if (matches(name, ((rd_file*)current->file)->name)) {
      return ((rd_file*)current->file);
    }
    current = current->next;
  }
  return NULL;
}

/*
 * count will equal the number of parent directories in path. E.g.:
 * path="/", count=0;
 * path="/var", count=0;
 * path="/var/log", count=1;
 */
char **get_dirs(const char *path, int *ret_count) {
  int path_len, i, count, flag, start, end;
  char ** file_names;

  if (!path) {
    return NULL;
  }

  path_len = strlen(path);
  count = count_occurences('/', path);
  file_names = (char**) malloc(sizeof(char*) * count+1);
  memset(file_names, 0, sizeof(char*) * count+1);

  for (i=0; i < count; i++) {
    file_names[i]=NULL;
  }

  count = -1;  /* when the for-loop ends, count will be, the number of "/" -1 */
  start = end = 0;
  for (i = 0; i < path_len; i++) {
    flag = 0;
    if ((*(path + i)) == '/') {
      end = i;
      count++;
      flag = 1;
    }
    if (!flag)
      continue;
    if (count == 0) {
      start = end;
      continue;
    }
    file_names[count - 1] = (char*) malloc(sizeof(char) * (end - start));
    memset(file_names[count - 1], 0, sizeof(char) * (end - start));
    memcpy(file_names[count - 1], path + start + 1, sizeof(char) * (end - start - 1));
    start = end;
  }
  file_names[count] = (char*) malloc(sizeof(char) * (path_len - start));
  memset(file_names[count], 0, sizeof(char) * (path_len - start));
  memcpy(file_names[count], path + start + 1, sizeof(char) * (path_len - start - 1));

  *ret_count = count;
  return file_names;
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

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * path   - path to the file
 * buffer - buffer to write the contents to
 * size   - size of buffer and also the amount of data to read
 * offset - offset (from the beginning of file)
 */
static int rd_read (const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
  rd_file *file = NULL;
  //int read_block_num=0; /* how many blocks to read */
  int current_offset = 0; // offset of the file during this reading, in byte
  int block_index_offset = 0; // the block index, from which this reading starts, starts from
  int block_byte_offset = 0; // the byte offset of block_index_offset, from which this reading starts
  int r_size = 0; /* real read size; the given size could be bigger than the file's actual size */
  int t_size = 0; /* local copy of size, will be modified */
  int i = 0;
  int buffer_offset = 0;
  int to_copy_block_num = 0; /* number of blocks to copy, excluding current block */
  int current_block_remaining = 0; /* current block's bytes after the offset */

  // sprintf(s, "r_read called, path:%s, size:%d, offset:%d, fi:%ld\n", path, size, offset, fi);

  if (!valid_path(path)) {
    return -ENOENT;
  }

  if (buffer == NULL || offset < 0) {
    DEBUG_PRINT("buffer is NULL or offset < 0\n");
    return -EPERM;
  }

  file = get_rd_file(path, REGULAR, root);
  if(!file) {
    return -ENOENT;
  }

  current_offset = offset;
  block_index_offset = current_offset / BLOCK_BYTES;
  block_byte_offset = (current_offset % BLOCK_BYTES);


  // sprintf(s, "r_read, current_offset:%d, block_index_offset:%d, block_byte_offset:%d\n",
  //     current_offset, block_index_offset, block_byte_offset);

  /* sometimes, size could very large, but the actual file's size is very small */
  if ((block_byte_offset + size/* 4095+1 */) <= BLOCK_BYTES ) { /* if only read from current block; */
    memcpy(buffer, file->blocks[block_index_offset]+block_byte_offset, size);
  } else { // if need to read from the remaining blocks
    current_block_remaining = BLOCK_BYTES - block_byte_offset ; /*4096-4095==1 bytes remained to be read;(index starts from 0!) */
    if (size > (file->bytes - current_offset)) { /* if the given size is bigger than the file's max possible read size(total-offset) */
      r_size = file->bytes - current_offset;
    } else {
      r_size = size;
    }
    t_size = r_size;

    /* to_copy_block_num: excluding current block */
    if (((r_size - current_block_remaining) % BLOCK_BYTES)!= 0) {
      if (r_size - current_block_remaining < BLOCK_BYTES) { // if only read from current block
        to_copy_block_num = 0;
      } else {
        to_copy_block_num = (r_size - current_block_remaining)/(BLOCK_BYTES + 1);
      }
    } else {
      to_copy_block_num = (r_size - current_block_remaining)/BLOCK_BYTES;
    }

    // sprintf(s, "r_read, r_size:%d, to_copy_block_num:%d\n", r_size, to_copy_block_num);

    // copy current block first; this copy is likely only part of the block
    memcpy(buffer, file->blocks[block_index_offset] + block_byte_offset, current_block_remaining);
    buffer_offset += current_block_remaining;
    t_size -= current_block_remaining;

    /* copy remaining bytes */
    for (i = 1; i <= to_copy_block_num; i++) {
      if ((block_index_offset+i) >= file->num_blocks || file->blocks[block_index_offset+i] == NULL ) {

        // sprintf(s, "r_read, read error, (block_index_offset[%d]+i[%d])>=r_file->block_number[%d], "
        //     "or r_file->blocks[block_index_offset+i]==NULL.  t_size:%d\n", block_index_offset, i, r_file->block_number, t_size);
        continue; /* try to skip this unknown error */
      }
      if (t_size > BLOCK_BYTES) { /* copy whole block */
        memcpy(buffer + buffer_offset, file->blocks[block_index_offset+i], BLOCK_BYTES);
        buffer_offset += BLOCK_BYTES;
        t_size -= BLOCK_BYTES;
      } else {
        memcpy(buffer + buffer_offset, file->blocks[block_index_offset+i], t_size);
      }
    }
  }
  return r_size;
}


// TODO: add mode
static int rd_create (const char *path, mode_t mode, struct fuse_file_info *fi) {
  char **file_names;
  int i, count, flag;
  rd_file *file, *parent_file, *current_file;
  int ret_val = 0;

  // sprintf(s, "r_create called, path:%s, fi:%ld\n", path, fi);

  if (path ==NULL || matches(path, "/") || ends_with(path, "/")) {
    // chat_log_level(ramdisk_engine->lgr, "r_create called, path is NULL, or only /, or ended with /\n", LEVEL_ERROR);
    return -EPERM;
  }

  file_names = get_dirs(path, &count);
  if( file_names==NULL ){
    // chat_log_level(ramdisk_engine->lgr, "r_create called, cannot create file_names\n", LEVEL_ERROR);
    return -EPERM;
  }

  // check parent dir exist, and create file
  if (count != -1) {
    if (count != 0) { //if more than one level dir
      for (i=0; i<=count-1; i++) { //this for only checks if parent does not exist, doesn't create dir
        if (i == 0) {
          parent_file = get_file(file_names[i], root->files);
          if (parent_file == NULL || parent_file->type == REGULAR) {
            // sprintf(s, "r_create, dir:%s doesn't exist or it's REG-file, cannot create file, 1\n", file_names[i]);
            ret_val = -ENOENT;
            break;
          }
          continue;
        }
        current_file = get_file(file_names[i], parent_file->files);
        if (current_file == NULL || current_file->type == REGULAR) {
          // sprintf(s, "r_create, dir:%s doesn't exist or it's REG-file, cannot create file, 2\n", file_names[i]);
          ret_val = -ENOENT;
          break;
        }
        parent_file = current_file;
      }

      if (ret_val == 0){ // if parent-dir check pass, create file
        file = create_rd_file(file_names[count], parent_file->name );
        if (file != NULL) {
          file->parent = parent_file;
          push(parent_file->files, file);
          // sprintf(s, "r_create, file:%s is created, and added to %s-dir\n", r_file->name, r_file_p->name);
        } else {
          ret_val = -EPERM;
        }
      }
    } else { // count==0
      file = create_rd_file(file_names[count], "/"); // create new file under root directory
      if (file != NULL) {
        push(root->files, file);
        // sprintf(s, "r_create, file:%s is created, and added to root-dir\n", r_file->name);
      } else {
        ret_val = -EPERM;
      }
    }
  } else {
    ret_val = -ENOENT;
    // chat_log_level(ramdisk_engine->lgr, "r_create, unknown mistakes, count is -1\n", LEVEL_ERROR);
  }

  // free files
  for (i=0; i<=count; i++) {
    free(file_names[i]);
  }
  free(file_names);
  return ret_val;
}

static int rd_getattr (const char *path, struct stat *stbuf) {
  char **file_names;
  int i, count, flag;
  rd_file *file, *parent_file, *current_file;
  int ret_val = 0;

  DEBUG_PRINT("rd_getattr:%s\n", path);

  if (path==NULL || ends_with(path, "/")){
    return -EPERM;
  }

  if (matches(path, "/")) {
    stbuf->st_atime = init_time;
    stbuf->st_mtime = init_time;
    stbuf->st_ctime = init_time;
    stbuf->st_size = DIRECTORY_BYTES;
    stbuf->st_mode = S_IFDIR | DEFAULT_DIRECTORY_PERMISSION;
    stbuf->st_nlink = 2;
    stbuf->st_uid = uid;
    stbuf->st_gid = gid;
    return 0;
  }

  file_names = get_dirs(path, &count);
  if (!file_names){
    DEBUG_PRINT("rd_getattr: no directories\n");
    return -EPERM;
  }

  // check for parent directory exist and get attributes if it exists
  if (count != -1) {
    if (count != 0) {
      for (i = 0; i <= count - 1; i++) {
        if (i == 0) {
          parent_file = get_file(file_names[i], root->files);
          if (parent_file==NULL || parent_file->type==REGULAR) {
            // directory doesn't exist or it's actually a REGULAR file
            ret_val = -ENOENT;
            break;
          }
          continue;
        }
        current_file = get_file(file_names[i], parent_file->files);
        if (current_file == NULL || current_file->type == REGULAR) {
          // directory doesn't exist or it's actually a REGULAR file
          ret_val = -ENOENT;
          break;
        }

        parent_file = current_file;
      } /* end of for */
      if (ret_val == 0) { /* if parent-dir check pass */
        file = get_file(file_names[count], parent_file->files);
        if (file == NULL) { /* if file exists */
          // file doesn't exist in the directory
          ret_val = -ENOENT;
        }
      }
    } else {  // count == 0
      file = get_file(file_names[count], root->files);
      if (file == NULL) {
        // file doesn't exist under root directory
        ret_val = -ENOENT;
      }
    }
    if (ret_val == 0) {
      stbuf->st_atime = init_time;
      stbuf->st_mtime = init_time;
      stbuf->st_ctime = init_time;
      stbuf->st_uid = uid;
      stbuf->st_gid = gid;

      if (file->type == DIRECTORY) {
        stbuf->st_size = DIRECTORY_BYTES;
        stbuf->st_mode = S_IFDIR | DEFAULT_DIRECTORY_PERMISSION;
        stbuf->st_nlink = 2;
      } else {
        stbuf->st_size = file->bytes;
        stbuf->st_mode = S_IFREG | DEFAULT_FILE_PERMISSION;
        stbuf->st_nlink = 1;
        stbuf->st_blksize = BLOCK_BYTES;
        stbuf->st_blocks = file->num_blocks;
      }
    }
  } else {
    ret_val = -ENOENT;
  }

  for (i=0; i<=count; i++) {
    free(file_names[i]);
  }
  free(file_names);
  return ret_val;
}


static struct fuse_operations operations = {
  .open      = rd_open,
  .flush     = rd_flush,  // close()
  .read      = rd_read,
  .create    = rd_create,
  // .write     = rd_write,    //==> ssize_t write(int filedes, const void * buf , size_t nbytes ) in POSIX
  // .unlink    = rd_unlink,
  // .opendir   = rd_opendir,
  // .readdir   = rd_readdir,
  // .mkdir     = rd_mkdir,
  // .rmdir     = rd_rmdir,
  .getattr   = rd_getattr
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

  gid = getgid();
  uid = getuid();
  init_time = time(NULL);

  return fuse_main(argc - 1, argv, &operations, NULL);
}


