#include <stdio.h>
#include "../mythread.h"

int n;

void t1(void * who)
{
  int i;

  printf("t%d start\n", (int)who);
  for (i = 0; i < n; i++) {
    printf("t%d yield\n", (int)who);
    MyThreadYield();
  }
  printf("t%d end\n", (int)who);
  MyThreadExit();
}

void t0(void * dummy)
{
  MyThreadCreate(t1, (void *)1);
  t1(0);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    return -1;
  n = atoi(argv[1]);
  MyThreadInit(t0, 0);
}

/////////////////////////////////////
// Output
/////////////////////////////////////
// t0(0) --> thread(t1(1)) and t1(0)
// thread(t(1)): "t1 start"
//             : "t1 yield" * n
// thread(t(1)): "t1 end"
//
// t1(0)       : "t0 start"
// t1(0)       : "t0 yield" * n
// t1(0)       : "t0 end"
