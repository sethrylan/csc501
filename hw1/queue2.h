#include <stddef.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Thread {
  ucontext_t ctx;
  struct Thread *parent;
  struct Queue *children;
  struct Thread *waiting_for;
} Thread;

typedef struct ThreadNode
{
  Thread *thread;
  struct ThreadNode* next;
} ThreadNode;

typedef struct Queue  /* FIFO queue */
{
  struct ThreadNode* head;
  struct ThreadNode* tail;
} Queue;
 
// typedef struct Queue {      
//   int capacity;
//   int size;
//   int front;
//   int rear;
//   Thread *elements;
// } Queue;


Queue* make_queue(void)
{
  Queue *q = malloc(sizeof(Queue));
  q->head = NULL;
  q->tail = NULL;
  return q;
}

int is_empty(Queue *q)
{
  return ((q == NULL) || ((q->head == NULL) && (q->tail == NULL)));
}

void die (const char *msg)
{
  perror(msg);
  exit(1);
}

Thread* dequeue(Queue *q)
{
  ThreadNode* head = NULL;
  ThreadNode* temp = NULL;
  Thread *thread = NULL;
  if (is_empty(q))
  {
    printf("List is empty\n");
    return NULL;
  }
  else if (NULL == q->head || NULL == q->tail)
  {
    printf("There is something seriously wrong with your list\n");
    printf("One of the head/tail is empty while other is not \n");
    return NULL;
  }
  head = q->head;
  thread = head->thread;
  q->head = head->next;
  temp = head;
  free(temp);
  if (q->head == NULL)
  {
    q->tail = q->head;   /* The element tail was pointing to is free(), so we need an update */
  }
  return thread;
}

Queue* enqueue(Queue *q, Thread *thread)
{
  ThreadNode *node = malloc( 1 * sizeof(ThreadNode) );
  node->thread = thread;
  node->next = NULL;

  if (NULL == q)
  {
    printf("Queue not initialized\n");
    free(node);
    return q;
  }
  else if (is_empty(q))
  {
    q->head = node;
    q->tail = node;
    return q;
  }
  else if (NULL == q->head || NULL == q->tail)
  {
    fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
    free(node);
    return NULL;
  }
  else
  {
    /* printf("List not empty, adding element to tail\n"); */
    q->tail->next = node;
    q->tail = node;
  }

  return q;
}

void print_node (const ThreadNode *node)
{
  if (node)
  {
    // printf("Thread = %d\n", p->num);
  }
  else
  {
    // printf("Can not print NULL struct \n");
  }
}

void print_queue (const Queue *q)
{
  ThreadNode *node = NULL;
  if (q)
  {
    for (node = q->head; node != NULL; node = node->next )
    {
      print_node(node);
    }
  }
}

Queue* free_queue(Queue *q)
{
  while(q->head)
  {
    dequeue(q);
  }
  return q;
}

int contains(Queue *q, Thread *thread)
{
  ThreadNode *node = NULL;
  for (node = q->head; node != NULL; node = node->next )
  {
    if (node->thread == thread)
    {
      return 1;
    }
  }
  return 0;
}

void remove_node(Queue *q, Thread *thread)
{
  ThreadNode *last = NULL;
  ThreadNode *node = NULL;
  for (node = q->head; node != NULL; node = node->next )
  {
    if (node->thread == thread)
    {
      if(node == q->head && node == q->tail)
      {
        q->head = q->tail = NULL;
      }
      else if (node == q->tail)
      {
        q->tail = last;
        q->tail->next  = NULL;
      }
      else if (node == q->head)
      {
        q->head = node->next;
      }
      else
      {
        last->next = node->next;
      }
    }
    last = node;
  }
}



