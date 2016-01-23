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

#define _XOPEN_SOURCE 700 // see https://lists.apple.com/archives/darwin-dev/2008/Feb/msg00015.html
#include "mythread.h"
#include "queue.h"
#include <stdio.h>

Queue *ready_queue;
Queue *blocked_queue;

Thread *currentThread;
Thread *initThread;

static ucontext_t initProcesssContext;

Thread* make_thread (void(*start_funct)(void *), void *args)
{
  printf("start make_thread\n");
  Thread *thread = (Thread *)malloc(sizeof(Thread));
  thread->children = make_queue(1000);
  thread->waiting_for = NULL;
  thread->parent = NULL;

  ucontext_t *context = (ucontext_t *)malloc(sizeof(ucontext_t));

  if(getcontext(context) == -1) {
    printf("Could not get context\n");
    exit(1);
  }

  (context->uc_stack).ss_sp = (char *)malloc(4048*sizeof(char));
  (context->uc_stack).ss_size = 4048;
  context->uc_link = &(currentThread->ctx);
  makecontext(context, (void (*)()) start_funct, 1, args);
  thread->ctx = *context;

  return thread;
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
  printf("start of MyThreadExit\n");
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
  printf("MyThreadInit\n");
  ready_queue = make_queue(1000);
  blocked_queue = make_queue(1000);

  // initProcesssContext = (ucontext_t *)malloc(sizeof(ucontext_t));

  currentThread = make_thread(start_funct, args);
  initThread = currentThread;

  // save current thread context in initProcesssContext, and make currentThread context active
  swapcontext(&initProcesssContext, &(currentThread->ctx));

  // set parent thread to 

  //    getcontext(&(mainthread->threadctx));

  //    makecontext(&(mainthread->threadctx),(void(*)(void))start_funct,1,args);

  // add to ready queue
  // addToQueue(readyQ, mainthread);

  return;
}



/*........................ end of mythread.c .....................................*/
