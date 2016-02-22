typedef struct node {
  char *value;
  struct node *next;
} node;

node* make_node(char *value);

void print_list(node *head);

// add node to end of list
void push(node *head, char *value);

// remove head and return value
char* pop(node **head);
