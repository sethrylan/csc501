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
  Q = malloc(sizeof(Queue));
  Q->elements = malloc(sizeof(Thread)*max_size);
  Q->size = 0;
  Q->capacity = max_size;
  Q->front = 0;
  Q->rear = -1;
  return Q;
}

void die (const char *msg)
{
  perror(msg);
  exit(1);
}

Thread* pop (Queue *q) {
  Thread *result = NULL;
  if(q->size != 0) {
    result = &q->elements[q->front];
    q->size--;

    q->front = (q->front+1) % q->capacity;
    // q->front++;
    /* circle to front of queue */
    // if(q->front == q->capacity) {
    //   q->front = 0;
    // }
  }
  return result;
}

void push (Queue *q, Thread element) {
  if(q->size == q->capacity) {
    // todo: expandCapacity
    die("queue full\n");
  }
  q->size++;

  q->rear = (q->rear+1) % q->capacity;
  // q->rear = q->rear + 1;
  // if(q->rear == q->capacity) {        // circle queue
  //   q->rear = 0;
  // }
  q->elements[q->rear] = element;
  return;
}

void removeElement(Queue *q, Thread *thread) {
  Queue *new_queue = make_queue(q->capacity);
  while (q->size > 0)
  {
    push(new_queue, *pop(q));
  }
  free(q->elements);
  free(q);
  q = new_queue;
  // int found = 0;
  // for(int i=q->front; i<q->capacity; i++)
  // {
  //   if (&(q->elements[i]) == thread)
  //   {
  //     q->size--;
  //     q->rear = (q->rear-1) % q->capacity;
  //     for (int j=i; j<q->capacity-1; j++)
  //     {
  //       q->elements[j] = q->elements[j+1];
  //     }
  //   }
  // }
  return;
}

int contains(Queue *q, Thread *thread) {
  int i;
  for(i=0; i<q->capacity; i++)
  {
    if (&(q->elements[i]) == thread) {
      return 1;
    }
  }
  return 0;
}


// Thread new_tree_node (long offset, int level) {
//   tree_node tree_node;
//   tree_node.offset = offset;
//   tree_node.level = level;
//   return tree_node;
// }

