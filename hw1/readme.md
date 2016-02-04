* * *

### Description:

Implement a non pre-emptive user-level threads library _mythread.a_ with the following routines.

### Description:

To build:

```make clean lib```

To test:

``make clean test```

#### Thread routines.

MyThread **MyThreadCreate** (void(*start_funct)(void *), void *args)
This routine creates a new _MyThread_. The parameter _start_func_ is the function in which the new thread starts executing. The parameter _args_ is passed to the start function. This routine does _not_ pre-empt the invoking thread. In others words the parent (invoking) thread will continue to run; the child thread will sit in the ready queue.

void **MyThreadYield**(void)
Suspends execution of invoking thread and yield to another thread. The invoking thread remains ready to execute—it is not blocked. Thus, if there is no other ready thread, the invoking thread will continue to execute.

int **MyThreadJoin**(MyThread thread)
Joins the invoking function with the specified child thread. If the child has already terminated, do not block. Note: A child may have terminated without the parent having joined with it. Returns 0 on success (after any necessary blocking). It returns -1 on failure. Failure occurs if specified thread is not an immediate child of invoking thread.

void **MyThreadJoinAll**(void)
Waits until all children have terminated. Returns immediately if there are no _active_ children.</dd>


void **MyThreadExit**(void)
Terminates the invoking thread. _**Note:**_ all _MyThread_s are required to invoke this function. Do not allow functions to “fall out” of the start function.



#### Semaphore routines.

MySemaphore **MySemaphoreInit**(int initialValue)
Create a semaphore. Set the initial value to _initialValue_, which must be non-negative. A positive initial value has the same effect as invoking _MySemaphoreSignal_ the same number of times.

void **MySemaphoreSignal**(MySemaphore sem)
Signal semaphore _sem_. The invoking thread is not pre-empted.

void **MySemaphoreWait**(MySemaphore sem)
Wait on semaphore _sem_.

int **MySemaphoreDestroy**(MySemaphore sem)
Destroy semaphore _sem_. Do not destroy semaphore if any threads are blocked on the queue. Return 0 on success, -1 on failure.


#### Unix process routines.

The Unix process in which the user-level threads run is not a _MyThread_. Therefore, it will not be placed on the queue of _MyThreads._ Instead it will create the first _MyThread_ and relinquish the processor to the _MyThread_ engine.

The following routine may be executed only by the Unix process.


void **MyThreadInit** (void(*start_funct)(void *), void *args)
This routine is called before any other _MyThread_ call. It is invoked only by the Unix process. It is similar to invoking _MyThreadCreate_ immediately followed by _MyThreadJoinAll_. The _MyThread_ created is the oldest ancestor of all _MyThread_s—it is the “main” _MyThread_. This routine can only be invoked once. It returns when there are no threads available to run (i.e., the thread ready queue is empty.


#### Notes:

*   Use a FIFO (first-in, first-out) scheduling policy for threads on the ready queue.
*   Make all the routines available via a library. Use the _ar_ command to make a library. It will be used similar to this:

    > <kbd>ar rcs mythread.a file1.o file2.o file3.o</kbd>

*   All programs that use this library will include a header file (_mythread.h_) that defines all the interfaces and data structures that are needed—but no more.
*   This library does not have to be _thread-safe_ and will only execute in a single-threaded (OS) process. Assume that an internal thread operation cannot be interrupted by another thread operation. (E.g., _MyThreadExit_ will not be interrupted by _MyThreadYield_.) That means that the library does not have to acquire locks to protect the **internal** data of the thread library. (A user may still have to acquire semaphores to protect user data.)
*   The interface only allows one parameter (a **void ***) to be passed to a _MyThread_. This is sufficient. One can build wrapper functions that _pack_ and _unpack_ an arbitrary parameter list into an object pointed to by a **void ***. This [program](merge.c) gives an example of that. It is not a complete program—some of the logic is absent. It is only provided to illustrate how to use the interface.
*   Allocate at least 8KB for each thread stack.

* * *

### Turn In:

Be sure to turnin **all** the files needed. Include a <tt>Makefile</tt> that creates a library (a ".a" file) called <tt>mythread.a</tt>. An appropriate penalty will be assessed if this is not so.

Also, denote any resources used--including other students--in a file named **REFERENCES**.

* * *

### Resources:

Here is the [header](mythread.h) file for the library. This file is the _de facto_ interface for the library. **Do not edit it.**

Additionally, here is a [program](fib.c) that uses the library. **The program comes with no warranty at all.** It is provided as an illustration only.

Some [slides](mythread.pdf) describing the assignment.

* * *

### Testing:

This assignment will be compiled and tested on a VCL Linux machine.

* * *

### Notes:

1.  **Do not** use the _pthread_ library or any similar threading package.
2.  Instead use the “context” routines: _getcontext(2), setcontext(2), makecontext(3)_, and _swapcontext(3)_
3.  During testing many processes (or threads) will be created on machines used by others. Take care when testing.
4.  [Test programs](mythread_tests.tar.gz) Here are four example test cases. We think they are correct. However, there is no warranty. Alert us if you find any mistakes. We will use many more test cases during grading; do not let success on these codes lull you in to complacency.
5.  The code will be tested on the VCL image named [here.](submit.html)

* * *

### Grading:

The weighting of this assignment is given in [policies](../../policies.html#grading).

#### Extra Credit

Extra credit of 5% is available. Two restrictions were designed into the assignment to keep it simple: (1) Each thread must end in _MyThreadExit_ and (2) The initial "UNIX" process context is not used as a thread context. To receive the extra credit, students must provide the following work-arounds to those restrictions. Specifically, the library must do the following.

1.  Work correctly when threads do not end with _MyThreadExit_.
2.  Create a rountine _int MyThreadInitExtra(void)_ that initializes the threads package **and** converts the UNIX process context into a MyThread context.

    > <pre>  ...
    > 
    >   MyThreadInitExtra(...);
    >   // code here runs in thread context.
    >   // it is the root or "oldest ancestor" thread
    >   MyThreadCreate(...); // Creates a new thread
    >   MyThreadYield(...);  // Yield to new thread just created
    > 
    >   ...
    >   </pre>

3.  Submit a second header file (mythreadextra.h) that contains everything in the original header plus the definition of MyThreadInitExtra().


ssh <unityID>@remote-linux.eos.ncsu.edu
# enter unity password
