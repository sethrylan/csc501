#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

node* make_node(char *value) {
  node *head = malloc(sizeof(node));
  head->value = strdup(value);
  head->next = NULL;
  return head;
}

void print_list(node *head) {
  node *current = head;
  while (current != NULL) {
    printf("%s\n", current->value);
    current = current->next;
  }
}

// add node to end of list
void push(node *head, char *value) {
  node *current = head;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = make_node(value);
}

// remove head and return value
char* pop(node **head) {
  char* return_value = NULL;
  node *next_node = NULL;
  if (*head != NULL) {
    next_node = (*head)->next;
    return_value = strdup((*head)->value);
    free(*head);
    *head = next_node;
  }
  return return_value;
}

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
