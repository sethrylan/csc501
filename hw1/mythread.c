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
#include "queue.h"
#include <signal.h>
#include <stdlib.h>

Queue *ready_queue;
Queue *blocked_queue;

Thread *current_thread;
Thread *init_thread;

static ucontext_t init_context;

Thread* make_thread (void(*start_funct)(void *), void *args, ucontext_t *uc_context)
{
  Thread *thread = (Thread *)malloc(sizeof(Thread));
  thread->children = make_queue(1000);
  thread->waiting_for = NULL;
  thread->parent = NULL;

  ucontext_t *context = (ucontext_t *)malloc(sizeof(ucontext_t));

  if(getcontext(context) == -1) {
    die("getcontext failed\n");
  }

  (context->uc_stack).ss_sp = (char *)malloc(MINSIGSTKSZ * sizeof(char));
  (context->uc_stack).ss_size = MINSIGSTKSZ;
  context->uc_link = uc_context;
  makecontext(context, (void (*)()) start_funct, 1, args);
  thread->ctx = *context;

  return thread;
}

void free_thread(Thread *thread) {
  free((thread->ctx).uc_stack.ss_sp);
  free(thread->children);
  // thread->children = NULL;
  // thread->parent = NULL;
  free(thread);
  // thread = NULL;
}

Thread* get_next_thread() {
  Thread *next = pop(ready_queue);
  if(next == NULL) {
    setcontext(&init_context);
    return NULL;
  }
  else {
    return next;
  }
}

// Create a new thread.
MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
  return 0;
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
  return;
}

// Terminate invoking thread
void MyThreadExit (void)
{
  Thread *this = current_thread;

  // remove parent from blocked_queue
  // update children

  current_thread = get_next_thread();
  free_thread(this);

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
  ready_queue = make_queue(1000);
  blocked_queue = make_queue(1000);

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
