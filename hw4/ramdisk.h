
typedef enum {REGULAR, DIRECTORY} rd_file_type;
typedef enum {FALSE, TRUE} boolean;

#define BYTES_PER_BLOCK         4096
#define DIRECTORY_BYTES         4096

#define DEFAULT_DIRECTORY_PERMISSION 0777
#define DEFAULT_FILE_PERMISSION      0777

typedef struct node {
  void *file;
  struct node *next;
} node;

typedef struct rd_file {
  rd_file_type type;  // REGULAR or DIRECTORY

  // common attributes
  char *name;
  char *path;
  struct rd_file *parent;
  size_t size;    // size in bytes

  // REGULAR attributes
  char *data;
  boolean opened;

  // DIRECTORY attributes
  node *files;
} rd_file;

char* get_rd_file_path(rd_file *file) {
  char *result;
  asprintf(&result, "%s/%s", file->path, file->name);
  return result;
}

rd_file* create_rd_file(char *name, char *path, rd_file_type type) {
  rd_file *file;
  if (name == NULL || path == NULL){
    return NULL;
  }

  file = (rd_file*)malloc(sizeof(rd_file));
  file->type = type;
  file->name = strdup(name);
  file->path = strdup(path);
  file->files = NULL;
  file->opened = FALSE;
  file->parent = NULL;

  if (type == REGULAR) {
    file->size = 0;
  } else { // DIRECTORY file
    file->size = DIRECTORY_BYTES;
  }

  return file;
}

void free_file(rd_file *file) {
  if (file) {
    free(file->data);
    free(file->name);
    free(file->path);
    free(file);
  }
}


/////// LIST ///////

node* make_node(rd_file *file);

// add node to end of list
void push(node *head, rd_file *file);

// remove item from list and return head of list; also frees the item/file
node* delete_item(node* head, rd_file *item);

node* make_node(rd_file *file) {
  node *head = malloc(sizeof(node));
  head->file = file;
  head->next = NULL;
  return head;
}

// add node to end of list
void push(node *head, rd_file *file) {
  node *current = head;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = make_node(file);
}

node* delete_item(node* head, rd_file *item) {
  node *next;
  if (head == NULL) {
    return NULL;
  } else if ((rd_file*)head->file == item) { // found item to delete
    next = head->next;
    free_file(head->file);
    free(head);
    return next;
  } else {                                   // not found, so recurse to continue searching
    head->next = delete_item(head->next, item);
    return head;
  }
}

// remove head and return value
// rd_file* pop(node **head) {
//   rd_file *return_value = NULL;
//   node *next_node = NULL;
//   if (*head != NULL) {
//     next_node = (*head)->next;
//     return_value = (*head)->file;
//     free(*head);
//     *head = next_node;
//   }
//   return return_value;
// }

// int remove_last(node_t * head) {
//     int retval = 0;
//     /* if there is only one item in the list, remove it */
//     if (head->next == NULL) {
//         head->val
//         free(head);
//         head = NULL;
//         return retval;
//     }

//     node_t * current = head;

//     while (current->next->next != NULL) {
//         current = current->next;
//     }
// }

