#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fuse.h>
#include <errno.h>
#include <unistd.h>
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
// http://www.cs.cmu.edu/~./fp/courses/15213-s07/lectures/15-filesys/index.html

// TODO: support 'cat > file && cat > file'

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
  int path_length, count;
  char **file_names;

  if (!path) {
    return NULL;
  }

  path_length = strlen(path);
  count = count_occurences('/', path);
  file_names = calloc(count + 1, sizeof(char*));

  int start = 0, end = 0, found = 0;
  count = -1;
  for (int i = 0; i < path_length; i++) {
    found = 0;
    if ((*(path + i)) == '/') {
      end = i;
      count++;
      found = 1;
    }
    if (!found) {
      continue;
    }
    if (count == 0) {
      start = end;
      continue;
    }
    file_names[count - 1] = calloc(end - start, sizeof(char));
    strncpy(file_names[count - 1], path + start + 1, end - start - 1);
    start = end;
  }
  file_names[count] = calloc(path_length - start, sizeof(char));
  strncpy(file_names[count], path + start + 1, path_length - start - 1);
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


// TODO: remove unused file_type
rd_file* get_rd_file (const char *path, rd_file *root) {
  DEBUG_PRINT("get_rd_file(): %s\n", path);
  int count;
  char **file_names = get_dirs(path, &count);
  rd_file *parent_file = get_parent_directory(path, file_names, count);
  rd_file *file = get_file(file_names[count], parent_file->files);
  free_char_list(file_names, count);
  return file;
}

int rd_opendir (const char *path, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_opendir: %s\n", path);
  rd_file *file;

  if (matches(path, root->name)) {  // path is "/"
      return EXIT_SUCCESS;
  }
  // if (!path) {
  //   return -ENOENT;
  // } else if (matches(path, root->name)) {  // path is "/"
  //   return EXIT_SUCCESS;
  // } else if (ends_with(path, "/")){        // path is "/**/"
  //   return -ENOENT;
  // }

  file = get_rd_file(path, root);

  if (!file) {
    return -ENOENT;
  }

  if (file->type != DIRECTORY) {
    return -ENOTDIR;
  }

  return EXIT_SUCCESS;
}

static int rd_open (const char *path, struct fuse_file_info *fi){
  DEBUG_PRINT("rd_open called, path:%s, O_RDONLY:%d, O_WRONLY:%d, O_RDWR:%d, O_APPEND:%d, O_TRUNC:%d\n",
                              path, fi->flags&O_RDONLY, fi->flags&O_WRONLY, fi->flags&O_RDWR, fi->flags&O_APPEND, fi->flags&O_TRUNC);

  if (!valid_path(path)) {
    return -ENOENT;
  }

  rd_file *file = get_rd_file(path, root);

  if (!file) {
    return -ENOENT;
  }

  if (file->type == DIRECTORY && (fi->flags&O_RDONLY || fi->flags&O_RDWR || fi->flags&O_APPEND)) {
    return -EISDIR;
  }

  file->opened = TRUE;

  return EXIT_SUCCESS;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 */
int rd_flush (const char *path, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_flush(): %s\n", path);
  rd_file *file;
  if (!valid_path(path)) {
    return -ENOENT;
  }
  file = get_rd_file(path, root);
  if (!file) {
    return -ENOENT;
  }
  return EXIT_SUCCESS;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 */
static int rd_write (const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_write(): %s\n", path);
  rd_file *file;

  if (!valid_path(path)){
    return -ENOENT;
  }

  file = get_rd_file(path, root);
  if (!file) {
    return -ENOENT;
  }

  if (file->type != REGULAR) {
    // No error
  }

  if (offset > file->size) {
    return -EFBIG;
  }

  int size_after_write = offset + size;

  if (size_after_write > file->size) {
    int bytes_to_allocate = size_after_write - file->size;

    if(!memory_available(bytes_to_allocate)) {
      return -ENOSPC;
    }

    current_bytes += bytes_to_allocate;

    char *temp_file = calloc(bytes_to_allocate + file->size, sizeof(char));
    strncpy(temp_file, file->data, file->size);

    if (file->size > 0) {
      free(file->data);
    }

    file->data = temp_file;
    file->size =+ bytes_to_allocate + file->size;
  }
  int i,j=0;
  for(i = offset; i < (offset + size); i++) {
    file->data[i] = buffer[j];
    j++;
  }
  file->data[i];
  return size;
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
 * buffer - buffer to write the contents to
 * size   - size of buffer and the amount of data to read
 * offset - offset (from the beginning of file)
 */
static int rd_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_read(): %s\n", path);

  if (!valid_path(path)){
    return -ENOENT;
  }

  rd_file *file = get_rd_file(path, root);
  if (!file) {
    return -ENOENT;
  }

  if (file->type == DIRECTORY) {
    return -EISDIR;
  }

  if (file->data) {
    if (offset < file->size) {
      if ((offset + size) > file->size) {
        size = file->size - offset;
      }
      memcpy(buffer, file->data + offset, size);
      buffer[size] = '\0';
    } else {
      size = 0;
    }
  } else {
    size = 0;
  }
  return size;
}


// TODO: mode
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
  rd_file *file, *parent_file;

  if (!path){
    return -EPERM;
  }

  char **file_names = get_dirs(path, &count);
  if (matches(path, root->name)) {
    parent_file = root;
  } else {
    if (!file_names){
      return -EPERM;
    }
    parent_file = get_parent_directory(path, file_names, count);
  }

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
        statbuf->st_blksize = BYTES_PER_BLOCK;
        statbuf->st_blocks = DIRECTORY_BYTES / BYTES_PER_BLOCK;
      } else {
        statbuf->st_size = file->size;
        statbuf->st_mode = S_IFREG | DEFAULT_FILE_PERMISSION;
        statbuf->st_nlink = 1;
        statbuf->st_blksize = BYTES_PER_BLOCK;
        statbuf->st_blocks = div_round_up(file->size, BYTES_PER_BLOCK);
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
    return -ENOTDIR;
  }

  if (!memory_available(DIRECTORY_BYTES)) {
    return -ENOSPC;
  }

  char **file_names = get_dirs(path, &count);
  if (!file_names){
    return -ENOENT;
  }
  rd_file *parent_file = get_parent_directory(path, file_names, count);

  if (!parent_file) {
    return -ENOENT;
  } else {
    rd_file *file = get_file(file_names[count], parent_file->files);
    if (file) {
      // file already exists
      ret_val = -EEXIST;
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

  if (ret_val == EXIT_SUCCESS) {
    current_bytes += DIRECTORY_BYTES;
  }

  return ret_val;
}


int rd_unlink(const char * path) {
  DEBUG_PRINT("rd_unlink: %s\n", path);
  int count;
  rd_file *file;
  int ret_val = EXIT_SUCCESS;

  if (!path || matches(path, root->name)) {
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
      // TODO: return if file type is EISDIR
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


static int rd_rmdir (const char *path) {
  DEBUG_PRINT("rd_rmdir: %s\n", path);
  int ret_val = EXIT_SUCCESS;

  if (!path || matches(path, root->name)) {
    return -EPERM;
  }

  rd_file *file = get_rd_file(path, root);
  if (!file) {
    return -ENOENT;
  }

  if (file->type != DIRECTORY) {
    return -ENOTDIR;
  }

  if (file->files) {
    return -ENOTEMPTY;
  }

  rd_file *parent_file = file->parent;
  parent_file->files = delete_item(parent_file->files, file);
  current_bytes -= DIRECTORY_BYTES;
  return ret_val;
}

int rd_rename (const char *source, const char *dest) {
  DEBUG_PRINT("rd_rename: %s to %s\n", path, dest);

  int ret_val = EXIT_SUCCESS;
  // TODO
  return ret_val;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 */
static int rd_fgetattr (const char *path, struct stat *buf, struct fuse_file_info *fi) {
  DEBUG_PRINT("rd_fgetattr: %s\n", path);
  return rd_getattr(path, buf);
}

/**
 * Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 */
static int rd_statfs (const char* path, struct statvfs *st) {

}




static struct fuse_operations operations = {
  .getattr  = rd_getattr,
  .opendir  = rd_opendir,
  .readdir  = rd_readdir,
  .create   = rd_create,
  .utimens  = rd_utimens,
  .mkdir    = rd_mkdir,
  .access   = rd_access,
  .unlink   = rd_unlink,
  .open     = rd_open,
  .flush    = rd_flush,  // called on close()
  .write    = rd_write,
  .read     = rd_read,
  .rmdir    = rd_rmdir,
  .rename   = rd_rename,
  .fgetattr = rd_fgetattr,
  .statfs   = rd_statfs,
  // .truncate  = rd_truncate,
  // // .ftruncate = rd_ftruncate,
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
  current_bytes = 0;

  return fuse_main(argc - 1, argv, &operations, NULL);
}


