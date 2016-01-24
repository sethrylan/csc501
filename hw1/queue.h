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

typedef struct Queue {      /* FIFO queue */
  int capacity;
  int size;
  int front;
  int rear;
  Thread *elements;
} Queue;

Queue* make_queue (int max_size) {
  Queue *Q;
  Q = (Queue*)malloc(sizeof(Queue));
  Q->elements = (Thread *)malloc(sizeof(Thread)*max_size);
  Q->size = 0;
  Q->capacity = max_size;
  Q->front = 0;
  Q->rear = -1;
  return Q;
}

Thread* pop (Queue *q) {
  Thread *result = NULL;
  if(q->size != 0) {
    result = &q->elements[q->front];
    q->size--;
    q->front++;
    /* circle to front of queue */
    if(q->front == q->capacity) {
      q->front = 0;
    }
  }
  return result;
}

void push (Queue *q, Thread element) {
  if(q->size != q->capacity) {
    q->size++;
    q->rear = q->rear + 1;

    if(q->rear == q->capacity) {        // circle queue
      q->rear = 0;
    }
    q->elements[q->rear] = element;
  }
  return;
}

// Thread new_tree_node (long offset, int level) {
//   tree_node tree_node;
//   tree_node.offset = offset;
//   tree_node.level = level;
//   return tree_node;
// }

void die (const char *msg)
{
  perror(msg);
  exit(1);
}

