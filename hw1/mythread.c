#include "mythread.h"
#include "queue.h"
#include <signal.h>
#include <stdlib.h>
#include <execinfo.h>

Queue *ready_queue;
Queue *blocked_queue;

Thread *current_thread;
static ucontext_t init_context;

// Allocate and initialize new thread.
Thread* make_thread (void(*start_funct)(void *), void *args, ucontext_t *uc_link)
{
  Thread *thread = malloc(sizeof(Thread));
  thread->children = make_queue("children");
  thread->waiting_for = NULL;
  thread->parent = NULL;

  ucontext_t *context = malloc(sizeof(ucontext_t));

  // cf. getcontext error on Darwin: https://lists.apple.com/archives/darwin-dev/2008/Feb/msg00107.html
  if(getcontext(context) == -1) {
    die("getcontext failed\n");
  }

  // SIGSTKSZ is the "canonical size for a signal stack. It is judged to be sufficient for normal uses."
  context->uc_stack.ss_sp = malloc(SIGSTKSZ * sizeof(char));
  context->uc_stack.ss_size = SIGSTKSZ;
  context->uc_stack.ss_flags = 0;
  context->uc_link = uc_link;

  makecontext(context, (void (*)()) start_funct, 1, args);
  thread->ctx = context;

  DEBUG_PRINT("make_thread: %p \n", (void *)thread);

  return thread;
}

void free_thread(Thread *thread) {
  DEBUG_PRINT("free_thread %p\n", (void *)thread);
  free((thread->ctx->uc_stack).ss_sp);
  free(thread->ctx);
  free_queue(thread->children);
  thread->parent = NULL;
  free(thread);
}

// Returns the next thread from the ready queue. If there is no thread, then clean up and
// the set context to the initial process context
Thread* get_next_thread() {
  Thread *next = dequeue(ready_queue);
  if(!next) {
    DEBUG_PRINT("setting context to init context: %p \n", &init_context);
    free_queue(ready_queue);
    free_queue(blocked_queue);
    setcontext(&init_context);
  }
  return next;
}

// Create a new MyThread using start_func as the function to start executing. This routine
// does not pre-empt the invoking thread; the parent (invoking) thread will continue to
// run and the child thread will sit in the ready queue.
//
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

// Suspends execution of invoking thread and yield to another thread. The invoking thread
// remains ready to execute—it is not blocked. If there is no other ready thread, the
// invoking thread will continue to execute.
void MyThreadYield (void)
{
  Thread *temp = current_thread;
  enqueue(ready_queue, current_thread);
  current_thread = get_next_thread();
  swapcontext(temp->ctx, current_thread->ctx);
}

// Joins/blocks the invoking thread with the child thread passed as a parameter. Does not
// block if the child thread is already terminated.
// Returns 0 on success (after blocking), or -1 on failure if thread parameter is not a child thread
int MyThreadJoin (MyThread thread)
{
  Thread *t = (Thread *)thread;
  if (!contains(current_thread->children, t)) {
    return -1;
  } else {
    assert(t->parent = current_thread);
    current_thread->waiting_for = t;
    enqueue(blocked_queue, current_thread);
    current_thread = get_next_thread();
    swapcontext(t->parent->ctx, current_thread->ctx);
    return 0;
  }
}

// Blocks until all children have terminated. Returns immediately if there are no active children.
//
// If there there are children threads for the current thread, then:
// 1. Add current thread to blocked queue.
// 2. swapcontext() with next thread.
void MyThreadJoinAll (void)
{
  Thread *temp = current_thread;
  if (!is_empty(current_thread->children)) {
    enqueue(blocked_queue, current_thread);
    current_thread = get_next_thread();
    swapcontext(temp->ctx, current_thread->ctx);
  }
}


// Terminate invoking thread
// 1. Remove current_thread from it's parent thread's children
// 2. Unblock parent thread
// 3. Add parent thread to ready queue
// 4. Update current thread to next thread, and setcontext() for the new current thread
void MyThreadExit (void)
{
  DEBUG_PRINT("MyThreadExit: %p \n", (void *)current_thread);
  // Thread *temp = current_thread;
  Thread *parent = current_thread->parent;

  // remove current_thread from parent's children
  if(parent != NULL) {
    remove_node(parent->children, current_thread);
  }

  // unblock parent thread if it is blocked
  if(contains(blocked_queue, parent)) {
    if(is_empty(parent->children) || (parent->waiting_for == current_thread)) {
      remove_node(blocked_queue, parent);
      parent->waiting_for = NULL;
      enqueue(ready_queue, parent);
    }
  }

  // update children
  // ThreadNode *node = current_thread->children->head;
  // while (node)
  // {
  //   if (node->thread != init_thread)
  //   {
  //     node->thread->parent = init_thread;
  //     enqueue(init_thread->children, node->thread);
  //   }
  //   else
  //   {
  //     assert(0);
  //   }
  //   node = node->next;
  // }

  current_thread = get_next_thread();

  // TODO: cannot clean up until after switching context
  // if (temp) {
  //   free_thread(temp);
  // }

  if (current_thread) {
    setcontext(current_thread->ctx);
  }

  // will never get here
  assert(0);
}

// ****** SEMAPHORE OPERATIONS ******
// Create a semaphore and sets initial value, which must be > 0
MySemaphore MySemaphoreInit (int initialValue)
{
  DEBUG_PRINT("MySemaphoreInit(%d)\n", initialValue);
  Semaphore *semaphore = NULL;
  if (initialValue < 0) {
    perror("initialValue must be > 0");
  } else {
    semaphore = malloc(sizeof(Semaphore));
    semaphore->count = initialValue;
    semaphore->wait_queue = make_queue("wait_queue");
  }
  return (MySemaphore)semaphore;
}

// Signal a semaphore
void MySemaphoreSignal (MySemaphore sem)
{
  Semaphore *s = (Semaphore *)sem;
  s->count++;
  if (s->count <= 0) {
    Thread *thread = dequeue(s->wait_queue);
    enqueue(ready_queue, thread);
  }
  return;
}

// Wait on semaphore
void MySemaphoreWait (MySemaphore sem)
{
  Semaphore *s = (Semaphore *)sem;
  s->count--;
  if (s->count < 0) {
    Thread *temp = current_thread;
    enqueue(s->wait_queue, current_thread);
    current_thread = get_next_thread();
    swapcontext(temp->ctx, current_thread->ctx);
  }
  return;
}

// Destroy semaphore sem. Do not destroy semaphore if any threads are
// blocked on the queue. Return 0 on success, -1 on failure
int MySemaphoreDestroy (MySemaphore sem)
{
  Semaphore *s = (Semaphore *)sem;
  if (is_empty(s->wait_queue)) {
    free_queue(s->wait_queue);
    free(s);
    return 0;
  } else {
    return -1;
  }
}

// ****** CALLS ONLY FOR UNIX PROCESS ******
// Create and run the "main" thread
void MyThreadInit (void(*start_funct)(void *), void *args)
{
  ready_queue = make_queue("ready_queue");
  blocked_queue = make_queue("blocked_queue");
  current_thread = make_thread(start_funct, args, NULL);

  if (getcontext(&init_context) == -1) {
    die("getcontext failed\n");
  }

  // save current thread context in init_context, and make current_thread active
  swapcontext(&init_context, current_thread->ctx);

  return;
}
