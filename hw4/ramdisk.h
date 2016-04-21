
typedef enum {REGULAR, DIRECTORY} rd_file_type;
typedef enum {TRUE, FALSE} boolean;

#define BLOCK_BYTES             4048
#define INITIAL_BLOCKS_PER_FILE 4

struct node {
  void *file;
  struct node *next;
};

typedef struct rd_file {
  rd_file_type type;

  // common attributes
  char *name;
  char *path;
  struct rd_file *parent;

  // REGULAR attributes
  int bytes;
  int num_blocks;
  char **blocks;
  boolean opened;

  // DIRECTORY attributes
  struct node *files;
} rd_file;

char * get_rd_file_path(rd_file *file) {
  char *result;// = malloc(strlen(file->name) + strlen(file->path) + 2); // +2 for \0 and path separate
  asprintf(&result, "%s/%s", file->path, file->name);
  return result;
}

static rd_file* create_rd_file(char *name, char *path) {
  rd_file *file;
  if (name==NULL || path==NULL){
    return NULL;
  }

  file = (rd_file*)malloc(sizeof(rd_file));
  file->type = REGULAR;
  file->name = (char*)malloc(sizeof(char)*strlen(name)+1);
  file->path = (char*)malloc(sizeof(char)*strlen(path)+1);
  file->blocks = (char**)malloc( sizeof(char*)*INITIAL_BLOCKS_PER_FILE + 1);
  memset(file->name, 0, sizeof(char)*strlen(name)+1);
  memset(file->path, 0, sizeof(char)*strlen(path)+1);
  memcpy(file->name, name, sizeof(char)*strlen(name));
  memcpy(file->path, path, sizeof(char)*strlen(path));
  memset(file->blocks, 0, sizeof(char*)*INITIAL_BLOCKS_PER_FILE + 1 );
  file->files = NULL;
  file->num_blocks = INITIAL_BLOCKS_PER_FILE;
  file->bytes = 0;
  file->opened = FALSE;
  file->parent = NULL;
  return file;
}


/////// LIST ///////

typedef struct node node;

node* make_node(rd_file *file);

void print_list(node *head);

// add node to end of list
void push(node *head, rd_file *file);

// remove head and return value
char* pop(node **head);

node* make_node(rd_file *file) {
  node *head = malloc(sizeof(node));
  head->file = file;
  head->next = NULL;
  return head;
}

// void print_list(node *head) {
//   node *current = head;
//   while (current != NULL) {
//     printf("%s\n", current->file);
//     current = current->next;
//   }
// }

// add node to end of list
void push(node *head, rd_file *file) {
  node *current = head;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = make_node(file);
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

