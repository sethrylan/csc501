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

// Create a new thread.
MyThread MyThreadCreate(void(*start_funct)(void *), void *args) {
  return 0;
}

// Yield invoking thread
void MyThreadYield(void) {
  return;
}

// Join with a child thread
int MyThreadJoin(MyThread thread) {
  return 0;
}

// Join with all children
void MyThreadJoinAll(void) {
  return;
}

// Terminate invoking thread
void MyThreadExit(void) {
  return;
}

// ****** SEMAPHORE OPERATIONS ******
// Create a semaphore
MySemaphore MySemaphoreInit(int initialValue) {
  return 0;
}

// Signal a semaphore
void MySemaphoreSignal(MySemaphore sem) {
  return;
}

// Wait on a semaphore
void MySemaphoreWait(MySemaphore sem) {
  return;
}

// Destroy on a semaphore
int MySemaphoreDestroy(MySemaphore sem) {
  return 0;
}

// ****** CALLS ONLY FOR UNIX PROCESS ******
// Create and run the "main" thread
void MyThreadInit(void(*start_funct)(void *), void *args) {
  return;
}



/*........................ end of mythread.c .....................................*/
