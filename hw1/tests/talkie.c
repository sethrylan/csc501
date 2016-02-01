#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../mythread.h"

MySemaphore alice, bob;
char *message;

void alice_says() {
  int i;
  for (i=0; i<10; i++) {
    printf("alice here\n");
    MySemaphoreWait(alice); // wait for semaphore to be available
    // assert(strcmp(message, "from bob") == 0);
    printf("alice reads message = %s\n", message);
    strcpy(message, "from alice");
    MySemaphoreSignal(bob);    // tell bob to read message
  }
  MyThreadExit();
}

void bob_says() {
  // char *temp = malloc(100);
  // strcpy(temp, "bob says: ");

  int i;
  for (i=0; i<10; i++) {
    printf("bob here\n");
    MySemaphoreWait(bob);     // wait for sempahore to be available
    // assert(strcmp(message, "from alice") == 0);
    printf("bob reads message = %s\n", message);
    // strcat(temp, "hi ");
    strcpy(message, "from bob");
    MySemaphoreSignal(alice);    // tell alice to read message
  }
  MyThreadExit();
}

void talking() {
  MyThreadCreate(alice_says, NULL);
  MyThreadCreate(bob_says, NULL);
  MyThreadJoinAll();
}


int main(int argc, char **argv)
{
  if (argc < 3) {
    printf("usage: %s <n> <m>\n", argv[0]);
    exit(-1);
  }
  int n = atoi(argv[1]);
  int m = atoi(argv[2]);
  alice = MySemaphoreInit(n);
  bob = MySemaphoreInit(m);
  message = malloc(100 * sizeof(char));
  MyThreadInit(talking, NULL);
  MySemaphoreDestroy(alice);
  MySemaphoreDestroy(bob);
  free(message);
}
