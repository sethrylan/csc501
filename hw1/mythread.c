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
#include <execinfo.h>

Queue *ready_queue;
Queue *blocked_queue;

Thread *current_thread;
Thread *init_thread;

static ucontext_t init_context;

// Allocate and initialize new thread
Thread* make_thread (void(*start_funct)(void *), void *args, ucontext_t *uc_context)
{
  Thread *thread = malloc(sizeof(Thread));
  thread->children = make_queue("children");
  thread->waiting_for = NULL;
  thread->parent = NULL;

  ucontext_t *context = malloc(sizeof(ucontext_t));

  if(getcontext(context) == -1) {
    die("getcontext failed\n");
  }

  // SIGSTKSZ is the "canonical size for a signal stack. It is judged to be sufficient for normal uses."
  context->uc_stack.ss_sp = malloc(SIGSTKSZ * sizeof(char));
  context->uc_stack.ss_size = SIGSTKSZ;
  context->uc_stack.ss_flags = 0;
  context->uc_link = uc_context;

  makecontext(context, (void (*)()) start_funct, 1, args);
  thread->ctx = context;

  DEBUG_PRINT("make_thread: %p \n", (void *)thread);

  return thread;
}

void free_thread(Thread *thread) {
  DEBUG_PRINT("free_thread %p\n", (void *)thread);
  free((thread->ctx->uc_stack).ss_sp);
  free(thread->children);
  // thread->children = NULL;
  thread->parent = NULL;
  free(thread);
  // thread = NULL;
}

Thread* get_next_thread() {
  Thread *next = dequeue(ready_queue);
  if(!next) {
    DEBUG_PRINT("setting context to init context: %p \n", &init_context);
    setcontext(&init_context);
  }
  return next;
}

// Create a new thread.
// 1. Call make_thread to allocate and initialize with current_thread context as uc_link
// 2. Set parent thread of new thread to the current thread
// 3. Add new thread to parent thread's children
// 4. Add new thread to ready queue
MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
  Thread *thread = make_thread(start_funct, args, current_thread->ctx);
  DEBUG_PRINT("MyThreadCreate: %p\n", (void *)thread);
  thread->parent = current_thread;
  enqueue(current_thread->children, thread);
  enqueue(ready_queue, thread);
  return (MyThread)thread;
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
    swapcontext(temp->ctx, current_thread->ctx);
    return;
  }
}


// Terminate invoking thread
// 1. Remove current_thread from it's parent thread's children
// 2. Remove parent thread from blocked queue
// 3. Add parent thread to ready queue
// 4. Update current thread to next thread, and setcontext() for the new current thread
void MyThreadExit (void)
{
  Thread *temp = current_thread;
  Thread *parent = current_thread->parent;

  // remove current_thread from parent's children
  if(parent != NULL) {
    remove_node(parent->children, current_thread);
  }

  // remove parent from blocked_queue
  if(contains(blocked_queue, parent)) {
    if(is_empty(parent->children) || (parent->waiting_for == current_thread)) {
      remove_node(blocked_queue, parent);
      parent->waiting_for = NULL;
      enqueue(ready_queue, parent);
    }
  }

  // update children

  // for (int i=thread->children->front; i<thread->children->capacity)

  current_thread = get_next_thread();

  if (temp) {
    free_thread(temp);
  }

  setcontext(current_thread->ctx);

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
  ready_queue = make_queue("ready_queue");
  blocked_queue = make_queue("ready_queue");

  current_thread = make_thread(start_funct, args, NULL);
  init_thread = current_thread;

  if (getcontext(&init_context) == -1) {
    die("getcontext failed\n");
  }

  // save current thread context in initProcesssContext, and make current_thread context active
  swapcontext(&init_context, current_thread->ctx);

  // set parent thread to 

  //    getcontext(&(mainthread->threadctx));

  //    makecontext(&(mainthread->threadctx),(void(*)(void))start_funct,1,args);

  // add to ready queue
  // addToQueue(readyQ, mainthread);

  return;
}



/*........................ end of mythread.c .....................................*/
