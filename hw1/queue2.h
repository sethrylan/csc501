#include <stddef.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(...) printf( __VA_ARGS__ );
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif

typedef struct Thread {
  ucontext_t *ctx;
  struct Thread *parent;
  struct Queue *children;
  struct Thread *waiting_for;
} Thread;

typedef struct ThreadNode
{
  Thread *thread;
  struct ThreadNode *next;
} ThreadNode;

typedef struct Queue  // FIFO queue //
{
  struct ThreadNode *head;
  struct ThreadNode *tail;
  char *name;
} Queue;

// typedef struct Queue {
//   int capacity;
//   int size;
//   int front;
//   int rear;
//   Thread *elements;
// } Queue;

Queue* make_queue(char *name)
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

Thread* dequeue(Queue *q)
{
  ThreadNode* temp = q->head;
  Thread* thread = NULL;
  if (q->head == NULL) {
    // printf here causes
    // ==3938== Conditional jump or move depends on uninitialised value(s)
    // ==3938==    at 0x4F0A557: write (in /lib64/libc-2.12.so)
    // ==3938==    by 0x4EA0AD2: _IO_file_write@@GLIBC_2.2.5 (in /lib64/libc-2.12.so)
    // ==3938==    by 0x4EA0999: _IO_file_xsputn@@GLIBC_2.2.5 (in /lib64/libc-2.12.so)
    // ==3938==    by 0x4E78680: buffered_vfprintf (in /lib64/libc-2.12.so)
    // ==3938==    by 0x4E7321D: vfprintf (in /lib64/libc-2.12.so)
    // ==3938==    by 0x4E7E0E7: fprintf (in /lib64/libc-2.12.so)
    // ==3938==    by 0x40098E: dequeue (queue2.h:73)
    // ==3938==    by 0x400F49: get_next_thread (mythread.c:67)
    // ==3938==    by 0x401150: MyThreadExit (mythread.c:141)
    // ==3938==    by 0x40084E: t0 (one.c:7)
    // ==3938==    by 0x4E728EF: ??? (in /lib64/libc-2.12.so)
    // ==3938==    by 0x401241: MyThreadInit (mythread.c:192)
    // ==3938==  Uninitialised value was created by a stack allocation
    // ==3938==    at 0x4E78557: buffered_vfprintf (in /lib64/libc-2.12.so)
    DEBUG_PRINT("%s: queue is empty \n", q->name);
    return NULL;
  }
  assert(NULL != q->head);
  assert(NULL != q->tail);
  if (q->head == q->tail)
  {
    q->tail = q->head = NULL;
  }
  else
  {
    q->head = q->head->next;
  }
  thread = temp->thread;
  free(temp);
  DEBUG_PRINT("%s: dequeue thread pointer %p\n", q->name, (void *)thread);
  DEBUG_PRINT("%s: size = %d\n", q->name, size(q));
  return thread;
}

Queue* enqueue(Queue *q, Thread *thread)
{
  DEBUG_PRINT("%s: size = %d\n", q->name, size(q));

  ThreadNode *node = malloc( 1 * sizeof(ThreadNode) );
  node->thread = thread;
  node->next = NULL;

  if (is_empty(q))
  {
    q->head = node;
    q->tail = node;
  }
  else
  {
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
  free(q->name);
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
