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
// http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html

int memory_available (int bytes) {
  DEBUG_PRINT("memory_available(): %d (%ld / %ld)\n", bytes, current_bytes, max_bytes);
  return (current_bytes + bytes) <= max_bytes;
}

boolean valid_path(const char *path) {
  if (!path || matches(path, root->name) || ends_with(path, "/")) {
    DEBUG_PRINT("path is NULL or a directory\n");
    return FALSE;
  }
  return TRUE;
}

rd_file* get_file(const char *name, node *list){
  DEBUG_PRINT("get_file(): %s\n", name);
  node *current = list;
  while (current != NULL) {
    rd_file *current_file = (rd_file*)current->file;
    DEBUG_PRINT("get_file(): checking %s\n", current_file->name);

    if (matches(name, current_file->name)) {
      return current_file;
    }
    current = current->next;
  }
  DEBUG_PRINT("get_file(): returning NULL\n");
  return NULL;
}

/*
 * count will equal the number of parent directories in path. E.g.:
 * path="/", count=0;
 * path="/var", count=0;
 * path="/var/log", count=1;
 */
char** get_dirs(const char *path, int *ret_count) {
  int path_len, count, flag, start, end;
  char **file_names;

  if (!path) {
    return NULL;
  }

  path_len = strlen(path);
  count = count_occurences('/', path);
  file_names = (char**) malloc(sizeof(char*) * count+1);
  memset(file_names, 0, sizeof(char*) * count+1);

  for (int i = 0; i < count; i++) {
    file_names[i] = NULL;
  }

  count = -1;
  start = end = 0;
  for (int i = 0; i < path_len; i++) {
    flag = 0;
    if ((*(path + i)) == '/') {
      end = i;
      count++;
      flag = 1;
    }
    if (!flag) {
      continue;
    }
    if (count == 0) {
      start = end;
      continue;
    }
    file_names[count - 1] = (char*)malloc(end - start);  // TODO: replace with strdup
    memset(file_names[count - 1], 0, sizeof(char) * (end - start));
    memcpy(file_names[count - 1], path + start + 1, sizeof(char) * (end - start - 1));
    start = end;
  }
  file_names[count] = (char*)malloc(path_len - start);   // TODO: replace with strdup
  memset(file_names[count], 0, sizeof(char) * (path_len - start));
  memcpy(file_names[count], path + start + 1, sizeof(char) * (path_len - start - 1));

  *ret_count = count;
  return file_names;
}

rd_file* get_parent_directory (const char *path, char **file_names, const int count) {
  rd_file *parent_file = NULL, *current_file = NULL;
  if (!file_names) {
    DEBUG_PRINT("get_parent_directory: no directories\n");
    return NULL;
  }

  if (count < 0) {
    return NULL;
  }

  if (count == 0) {     // e.g., "/example";
    parent_file = root;
  } else {              // count > 0
    for (int i = 0; i <= count - 1; i++) {
      if (i == 0) {
        parent_file = get_file(file_names[i], root->files);
        if (parent_file == NULL || parent_file->type == REGULAR) {
          DEBUG_PRINT("parent_file is NULL or not a directory");
          return NULL;
        }
        continue;
      }
      current_file = get_file(file_names[i], parent_file->files);
      if (current_file == NULL || current_file->type == REGULAR) {
        DEBUG_PRINT("current_file is NULL or not a directory");
        return NULL;
      }
      parent_file = current_file;
    }
  }
  return parent_file;
}

rd_file* get_rd_file (const char *path, rd_file_type file_type, rd_file *root) {
  DEBUG_PRINT("get_rd_file(): %s\n", path);
  int count;
  char **file_names = get_dirs(path, &count);
  rd_file *parent_file = get_parent_directory(path, file_names, count);
  return get_file(file_names[count], parent_file->files);
}


int rd_opendir (const char *path, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_opendir: %s\n", path);
  rd_file *file;
  int ret_val = EXIT_SUCCESS;

  if (!path ) {
    return -ENOENT;
  } else if (matches(path, root->name)) {
    return EXIT_SUCCESS;
  } else if (ends_with(path, "/")){
    return -ENOENT;
  }

  file = get_rd_file(path, DIRECTORY, root);

  if (!file) {
    ret_val = -ENOENT;
  }

  return ret_val;
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

  return EXIT_SUCCESS;
}

int rd_flush (const char * path, struct fuse_file_info * fi) {
  DEBUG_PRINT("rd_flush(): %s\n", path);
  rd_file *file;
  if (!valid_path(path)) {
    return -ENOENT;
  }
  file = get_rd_file(path, REGULAR, root);
  if (!file) {
    return -ENOENT;
  }
  return EXIT_SUCCESS;
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
  DEBUG_PRINT("rd_read(): %s, %d, %d\n", path, size, offset);
  rd_file *file = NULL;
  //int read_block_num=0; /* how many blocks to read */
  int current_offset = 0; // offset of the file during this reading, in byte
  int block_index_offset = 0; // the block index, from which this reading starts, starts from
  int block_byte_offset = 0; // the byte offset of block_index_offset, from which this reading starts
  int r_size = 0; /* real read size; the given size could be bigger than the file's actual size */
  int t_size = 0; /* local copy of size, will be modified */
  int buffer_offset = 0;
  int to_copy_block_num = 0; /* number of blocks to copy, excluding current block */
  int current_block_remaining = 0; /* current block's bytes after the offset */


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
    if (size > (file->size - current_offset)) { /* if the given size is bigger than the file's max possible read size(total-offset) */
      r_size = file->size - current_offset;
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
    for (int i = 1; i <= to_copy_block_num; i++) {
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


// TODO: add mode and decrement memory
static int rd_create (const char *path, mode_t mode, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_create(): %s\n", path);
  int count, ret_val = EXIT_SUCCESS;

  if (!path || matches(path, root->name) || ends_with(path, "/")) {
    return -EPERM;
  }

  char **file_names = get_dirs(path, &count);
  if (!file_names) {
    return -EPERM;
  }
  rd_file *parent_file = get_parent_directory(path, file_names, count);

  if (!parent_file) {
    return -ENOENT;
  } else {
    rd_file *file = create_rd_file(file_names[count], parent_file->name, REGULAR);  // create file under parent directory
    if (file) {
      file->parent = parent_file;
      if (parent_file->files) {
        push(parent_file->files, file);
      } else {
        parent_file->files = make_node(file);
      }
    } else {
      ret_val = -EPERM;
    }
  }

  free_char_list(file_names, count);

  return ret_val;
}

static int rd_getattr (const char *path, struct stat *statbuf) {
  DEBUG_PRINT("rd_getattr(): %s\n", path);
  int count, ret_val = EXIT_SUCCESS;
  rd_file *file;

  if (!path || ends_with(path, "/")){
    return -EPERM;
  }

  char **file_names = get_dirs(path, &count);
  if (!file_names){
    return -EPERM;
  }
  rd_file *parent_file = get_parent_directory(path, file_names, count);

  if (!parent_file) {
    return -ENOENT;
  } else {
    if (matches(path, root->name)) {
      file = root;
    } else {
      file = get_file(file_names[count], parent_file->files);
    }
    if (!file) {
      DEBUG_PRINT("rd_getattr(): %s doesn't exist in %s\n", file_names[count], parent_file->name);
      ret_val = -ENOENT;
    } else {
      time(&(statbuf->st_atime));
      time(&(statbuf->st_mtime));
      time(&(statbuf->st_ctime));
      statbuf->st_uid = uid;
      statbuf->st_gid = gid;
      statbuf->st_dev = 2049;
      statbuf->st_ino = 14450705;
      statbuf->st_rdev = 0;

      if (file->type == DIRECTORY) {
        statbuf->st_size = DIRECTORY_BYTES;
        statbuf->st_mode = S_IFDIR | DEFAULT_DIRECTORY_PERMISSION;
        statbuf->st_nlink = 2;
        statbuf->st_blksize = BLOCK_BYTES;
        statbuf->st_blocks = 8;
      } else {
        statbuf->st_size = file->size;
        statbuf->st_mode = S_IFREG | DEFAULT_FILE_PERMISSION;
        statbuf->st_nlink = 1;
        statbuf->st_blksize = BLOCK_BYTES;
        statbuf->st_blocks = file->num_blocks;
      }
    }
  }

  free_char_list(file_names, count);

  return ret_val;
}


static int rd_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_readdir: %s\n", path);
  int count, ret_val = EXIT_SUCCESS;
  struct stat st;
  rd_file *file, *parent_file;
  node *node;

  if (!path) {
    return -EPERM;
  }

  char **file_names = get_dirs(path, &count);
  if (!file_names) {
    return -EPERM;
  }
  parent_file = get_parent_directory(path, file_names, count);

  if (!parent_file) {
    return -ENOENT;
  } else {
    if (matches(path, root->name)) {
      file = root;
    } else {
      file = get_file(file_names[count], parent_file->files);
    }
    if (!file || file->type == REGULAR) {
      DEBUG_PRINT("rd_readdir(): %s does not exist or is not a directory\n", file_names[count]);
      ret_val = -ENOENT;
    } else {
      node = file->files;
      while (node) {
        file = (rd_file*)node->file;

        memset(&st, 0, sizeof(struct stat));
        time(&(st.st_atime));
        time(&(st.st_mtime));
        time(&(st.st_ctime));

        st.st_uid = uid;
        st.st_gid = gid;
        if (file->type == REGULAR) {
          st.st_mode = S_IFREG | DEFAULT_FILE_PERMISSION;
          st.st_nlink = 1;
          st.st_size = file->size;
          /* st.st_blksize = BLOCK_SIZE;
          st.st_blocks = r_file->block_number; */
        } else {
          st.st_mode = S_IFDIR | DEFAULT_DIRECTORY_PERMISSION;
          st.st_nlink = 2;
          st.st_size = DIRECTORY_BYTES;
        }
        filler(buffer, file->name, &st, 0);
        node = node->next;
      }
    }

    free_char_list(file_names, count);
  }

  return ret_val;
}

static int rd_access (const char *path, int mask) {
  DEBUG_PRINT("rd_access: %s\n", path);
  return EXIT_SUCCESS;
}

static int rd_utimens (const char *path, const struct timespec tv[2]) {
  DEBUG_PRINT("rd_utimens: %s\n", path);
  return EXIT_SUCCESS;
}

int rd_mkdir (const char *path, mode_t mode) {
  DEBUG_PRINT("rd_mkdir: %s\n", path);
  int count, ret_val = EXIT_SUCCESS;

  if (!path || matches(path, root->name)) {
    return -EPERM;
  }

  if (!memory_available(DIRECTORY_BYTES)) {
    return -EFBIG;
  }

  char **file_names = get_dirs(path, &count);
  if (!file_names){
    return -EPERM;
  }
  rd_file *parent_file = get_parent_directory(path, file_names, count);

  if (!parent_file) {
    return -EPERM;
  } else {
    rd_file *file = get_file(file_names[count], parent_file->files);
    if (file != NULL) {
      ret_val = -EPERM;
    } else {
      rd_file *new_dir = create_rd_file(file_names[count], parent_file->name, DIRECTORY);  // e.g., "/"
      new_dir->parent = parent_file;
      if (parent_file->files) {
        push(parent_file->files, new_dir);
      } else {
        parent_file->files = make_node(new_dir);
      }
    }
  }

  free_char_list(file_names, count);

  if (ret_val ==0 ) {
    current_bytes += DIRECTORY_BYTES;
  }

  return ret_val;
}


int rd_unlink(const char * path) {
  DEBUG_PRINT("rd_unlink: %s\n", path);
  int count;
  rd_file *file;
  int ret_val = EXIT_SUCCESS;

  if (path == NULL || matches(path, root->name)) {
    return -EPERM;
  }

  char **file_names = get_dirs(path, &count);
  if (!file_names) {
    return -EPERM;
  }

  rd_file *parent_file = get_parent_directory(path, file_names, count);

  if (!parent_file) {
    return -EPERM;
  } else {
    rd_file *file = get_file(file_names[count], parent_file->files);
    if (!file) {
      ret_val = -EPERM;
    } else {
      DEBUG_PRINT("rd_unlink(): bytes before: %ld\n", current_bytes);
      DEBUG_PRINT("rd_unlink(): bytes freed: %ld\n", file->size);
      current_bytes -= file->size;
      DEBUG_PRINT("rd_unlink(): bytes after: %ld\n", current_bytes);
      parent_file->files = delete_item(parent_file->files, file);
    }
  }

  free_char_list(file_names, count);

  return ret_val;
}


static struct fuse_operations operations = {
  .getattr  = rd_getattr,
  .opendir  = rd_opendir,
  .readdir  = rd_readdir,
  .create   = rd_create,
  .utimens  = rd_utimens,
  .mkdir    = rd_mkdir,
  .access   = rd_access,
  .unlink    = rd_unlink,

  // // .rmdir     = rd_rmdir,
  // .open      = rd_open,
  // .flush     = rd_flush,  // close()
  // .read      = rd_read,
  // // .write     = rd_write,    //==> ssize_t write(int filedes, const void * buf , size_t nbytes ) in POSIX
  // // .fgetattr  = rd_fgetattr_wrapper, //==> int fstat(int pathname , struct stat * buf ) in POSIX
  // // .truncate  = rd_truncate_wrapper,
  // // .ftruncate = rd_ftruncate_wrapper,
  // // .rename   = rd_rename,
  // // .chmod    = rd_chmod,
  // // .chown    = rd_chown,
};

int main (int argc, char *argv[]) {
  // read parameters from command line
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <directory-path> <size-MB>\n", argv[0]);
    exit(1);
  }

  if (matches(argv[1], "-f")) {
    DEBUG_PRINT("-f used for foreground process.\n");
    mount_path = argv[2];
    max_bytes = atoi(argv[3]) * 1024 * 1024;
  } else {
    mount_path = argv[1];
    max_bytes = atoi(argv[2]) * 1024 * 1024;
  }

  DEBUG_PRINT("max_bytes = %ld\n", max_bytes);

  if (max_bytes < 0) {
    printf("size-MB cannot be less than 0\n");
    exit(1);
  }

  root = create_rd_file("/","", DIRECTORY);
  gid = getgid();
  uid = getuid();
  init_time = time(NULL);

  return fuse_main(argc - 1, argv, &operations, NULL);
}


