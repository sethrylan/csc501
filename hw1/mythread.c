/******************************************************************************
 *
 *  File Name........: mythread.c
 *
 *  Description......:
 *
 *  Created by vin on 11/21/06
 *
 *
 *****************************************************************************/

#include "mythread.h"
#include "queue2.h"
#include <signal.h>
#include <stdlib.h>

Queue *ready_queue;
Queue *blocked_queue;

Thread *current_thread;
Thread *init_thread;

static ucontext_t init_context;

Thread* make_thread (void(*start_funct)(void *), void *args, ucontext_t *uc_context)
{
  Thread *thread = malloc(sizeof(Thread));
  thread->children = make_queue();
  thread->waiting_for = NULL;
  thread->parent = NULL;

  ucontext_t *context = malloc(sizeof(ucontext_t));

  if(getcontext(context) == -1) {
    die("getcontext failed\n");
  }

  (context->uc_stack).ss_sp = malloc(MINSIGSTKSZ * sizeof(char));
  (context->uc_stack).ss_size = MINSIGSTKSZ;
  context->uc_link = uc_context;
  makecontext(context, (void (*)()) start_funct, 1, args);
  thread->ctx = *context;

  debug_print("make_thread %p\n", (void *)&thread);
  return thread;
}

void free_thread(Thread *thread) {
  free((thread->ctx).uc_stack.ss_sp);
  free(thread->children);
  // thread->children = NULL;
  thread->parent = NULL;
  free(thread);
  // thread = NULL;
}

Thread* get_next_thread() {
  Thread *next = dequeue(ready_queue);
  if(next == NULL) {
    debug_print("no next found\n", NULL);
    setcontext(&init_context);
    return NULL;
  } else {
    return next;
  }
}

// Create a new thread.
MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
  Thread *thread = make_thread(start_funct, args, &(current_thread->ctx));
  thread->parent = current_thread;
  enqueue(ready_queue, thread);
  enqueue(current_thread->children, thread);
  return thread;
}

// Yield invoking thread
void MyThreadYield (void)
{
  return;
}

// Join with a child thread
int MyThreadJoin (MyThread thread)
{
  return 0;
}

// Join with all children
void MyThreadJoinAll (void)
{
  if (is_empty(current_thread->children)) {
    return;
  } else {
    Thread *temp = current_thread;
    enqueue(blocked_queue, current_thread);
    current_thread = get_next_thread();
    swapcontext(&(temp->ctx), &(current_thread->ctx));
    return;
  }
}


// Terminate invoking thread
void MyThreadExit (void)
{
  // remove parent from blocked_queue
  Thread *parent = current_thread->parent;

  if(parent != NULL) {
    remove_node(parent->children, current_thread);
  }

  if(contains(blocked_queue, parent)) {
    if(is_empty(parent->children) || (parent->waiting_for == current_thread)) {
      remove_node(blocked_queue, parent);
      parent->waiting_for = NULL;
      enqueue(ready_queue, parent);
    }
  }

  // update children

  // for (int i=thread->children->front; i<thread->children->capacity)

  Thread *temp = current_thread;

  current_thread = get_next_thread();
  if (temp) {
    free_thread(temp);
  }

  setcontext(&(current_thread->ctx));

  return;
}

// ****** SEMAPHORE OPERATIONS ******
// Create a semaphore
MySemaphore MySemaphoreInit (int initialValue)
{
  return 0;
}

// Signal a semaphore
void MySemaphoreSignal (MySemaphore sem)
{
  return;
}

// Wait on a semaphore
void MySemaphoreWait (MySemaphore sem)
{
  return;
}

// Destroy on a semaphore
int MySemaphoreDestroy (MySemaphore sem)
{
  return 0;
}

// ****** CALLS ONLY FOR UNIX PROCESS ******
// Create and run the "main" thread
void MyThreadInit (void(*start_funct)(void *), void *args)
{
  ready_queue = make_queue();
  blocked_queue = make_queue();

  // init_context = (ucontext_t *)malloc(sizeof(ucontext_t));

  current_thread = make_thread(start_funct, args, NULL);
  init_thread = current_thread;

  if (getcontext(&init_context) == -1) {
    die("getcontext failed\n");
  }

  // save current thread context in initProcesssContext, and make current_thread context active
  swapcontext(&init_context, &(current_thread->ctx));

  // set parent thread to 

  //    getcontext(&(mainthread->threadctx));

  //    makecontext(&(mainthread->threadctx),(void(*)(void))start_funct,1,args);

  // add to ready queue
  // addToQueue(readyQ, mainthread);

  return;
}



/*........................ end of mythread.c .....................................*/
