#include <stddef.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stdout, __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

typedef struct Thread {
  ucontext_t *ctx;
  struct Thread *parent;
  struct Queue *children;
  struct Thread *waiting_for;
} Thread;

typedef struct ThreadNode {
  Thread *thread;
  struct ThreadNode *next;
} ThreadNode;

typedef struct Queue {      // FIFO queue
  struct ThreadNode *head;
  struct ThreadNode *tail;
  char *name;
} Queue;

typedef struct Semaphore {
  int count;
  struct Queue *wait_queue;
} Semaphore;

Queue* make_queue(const char *name)
{
  Queue *q = (Queue *)malloc(sizeof(Queue));
  q->name = malloc(strlen(name) + 1);
  strcpy(q->name, name);
  q->head = NULL;
  q->tail = NULL;
  return q;
}

int size(Queue *q) {
  int size = 0;
  ThreadNode *node = q->head;
  while(node) {
    size++;
    node = node->next;
  }
  return size;
}

int is_empty(Queue *q)
{
  return ((q->head == NULL) && (q->tail == NULL));
}

void die (const char *msg)
{
  perror(msg);
  exit(1);
}

// "every remove is from the head"
Thread* dequeue(Queue *q)
{
  ThreadNode* temp = q->head;
  Thread* thread = NULL;
  if (q->head == NULL) {
    DEBUG_PRINT("%s: queue is empty \n", q->name);
    return NULL;
  }
  assert(NULL != q->head);
  assert(NULL != q->tail);
  if (q->head == q->tail) {
    q->tail = q->head = NULL;
  } else {
    q->head = q->head->next;
  }
  thread = temp->thread;
  free(temp);
  DEBUG_PRINT("%s: dequeue thread pointer %p\n", q->name, (void *)thread);
  DEBUG_PRINT("%s: size = %d\n", q->name, size(q));
  return thread;
}

// "Every insert is at the tail"
Queue* enqueue(Queue *q, Thread *thread)
{
  DEBUG_PRINT("%s: size = %d\n", q->name, size(q));

  ThreadNode *node = malloc( 1 * sizeof(ThreadNode) );
  node->thread = thread;
  node->next = NULL;

  if (is_empty(q)) {
    q->head = node;
    q->tail = node;
  } else {
    assert(NULL != q->head);
    assert(NULL != q->tail);
    q->tail->next = node;
    q->tail = node;
  }

  DEBUG_PRINT("%s: enqueue thread pointer %p\n", q->name, (void *)thread);
  DEBUG_PRINT("%s: size = %d\n", q->name, size(q));

  return q;
}

void print_node (const ThreadNode *node)
{
  if (node)
  {
    // TODO: printf("Thread = %d\n", p->num);
  }
  else
  {
    // printf("Can not print NULL struct \n");
  }
}

void print_queue (const Queue *q)
{
  ThreadNode *node = NULL;
  if (q) {
    for (node = q->head; node != NULL; node = node->next ) {
      print_node(node);
    }
  }
}

void free_queue(Queue *q)
{
  DEBUG_PRINT("%s: freeing queue with %d remaining nodes\n", q->name, size(q));
  while(q->head) {
    dequeue(q);
  }
  free(q->name);
  free(q);
  return;
}

int contains(Queue *q, Thread *thread)
{
  ThreadNode *node = NULL;
  for (node = q->head; node != NULL; node = node->next ) {
    if (node->thread == thread) {
      return 1;
    }
  }
  return 0;
}

void remove_node(Queue *q, Thread *thread)
{
  ThreadNode *last = NULL;
  ThreadNode *node = NULL;
  for (node = q->head; node != NULL; node = node->next ) {
    if (node->thread == thread) {
      if(node == q->head && node == q->tail) {
        q->head = q->tail = NULL;
      } else if (node == q->tail) {
        q->tail = last;
        q->tail->next  = NULL;
      } else if (node == q->head) {
        q->head = node->next;
      } else {
        last->next = node->next;
      }
    }
    last = node;
  }
}
