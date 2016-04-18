
typedef enum {REGULAR, DIRECTORY} rd_file_type;
typedef enum {TRUE, FALSE} boolean;

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
  int block_size;
  char **blocks;
  boolean opened;

  // DIRECTORY attributes
  struct node *files;
} rd_file;


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

