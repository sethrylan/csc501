#include <stdio.h>
#include "../mythread.h"

int n, m;
int yield = 0;
int join = 0;

void t2(void * who)
{
  printf("t2 %d start\n", (int)who);
  MyThreadExit();
}

void t1(void *);

int makeThreads(char *me, void (*t)(void *), int many)
{
  MyThread T;
  int i;
  for (i = 0; i < many; i++) {
    printf("%s create %d\n", me, i);
    T = MyThreadCreate(t, (void *)i);
    if (yield)
      MyThreadYield();      
  }
  if (join)
    MyThreadJoin(T);
}

void t1(void * who_)
{
  char me[16];
  int who = (int)who_;
  sprintf(me, "t1 %d", who);
  printf("%s start\n", me);
  makeThreads(me, t2, m);
  printf("t1 %d end\n", who);
  MyThreadExit();
}

void t0(void * dummy)
{
  printf("t0 start\n");
  makeThreads("t0", t1, n);
  printf("t0 end\n");
  MyThreadExit();
}

int main(int argc, char *argv[])
{
  if (argc != 5) {
    return -1;
  }
  n = atoi(argv[1]);
  m = atoi(argv[2]);
  yield = atoi(argv[3]);
  join = atoi(argv[4]);

  MyThreadInit(t0, NULL);
}


/////////////////////////////////////
// Output
/////////////////////////////////////
// Mode: yield=0, join=0
// thread(t0(NULL)): "t0 start"
// thread(t0(NULL)): makeThreads("t0", t1, n)
// thread(t0(NULL)): "t0 create 0"
// thread(t0(NULL)): "t0 create 1"
// thread(t0(NULL)): "t0 create ..."
// thread(t0(NULL)): "t0 create n"
// thread(t0(NULL)): "t0 end"
// thread(t1(0))   : "t1 0 start"
// thread(t1(0))   : makeThreads("t1 0", t2, m)
// thread(t1(0))   : "t1 0 end"
// thread(t2(0..m)): "t2 <0..m> start"
// thread(t1(1))   : "t1 1 start"
// thread(t1(1))   : makeThreads("t1 0", t2, m)
// thread(t1(1))   : "t1 1 end"
// thread(t2(0..m)): "t2 <0..m> start"
/////////////////////////////////////
// Mode: n=2 m=2 yield=1, join=0
// thread(t0(NULL)): "t0 start"
// thread(t0(NULL)): makeThreads("t0", t1, n=2)
// thread(t0(NULL)): "t0 create 0"
// thread(t0(NULL)): <yield>
// thread(t1(0))   : "t1 0 start"
// thread(t1(0))   : makeThreads("t1 0", t2, 2)
// thread(t1(0))   : "t1 0 create 0"
// thread(t1(0))   : <yield>
// thread(t0(NULL)): "t0 create 1"
// thread(t0(NULL)): <yield>
// thread(t2(0))   : "t2 0 start"
// thread(t2(0))   : <exit>
// thread(t1(0))   : "t1 0 create 1"
// thread(t1(0))   : <yield>
// thread(t1(1))   : "t1 1 start"
// thread(t1(1))   : makeThreads("t1 1", t2, 2)
// thread(t1(1))   : "t1 1 create 0"
// thread(t1(1))   : <yield>
// thread(t0(NULL)): "t0 end"
// ...
/////////////////////////////////////
// Mode: n=2 m=2 yield=0, join=1
// thread(t0(NULL)): "t0 start"
// thread(t0(NULL)): makeThreads("t0", t1, n=2)
// thread(t0(NULL)): "t0 create 0"
// thread(t0(NULL)): "t0 create 1"
// thread(t0(NULL)): <join>
// thread(t1(0))   : "t1 0 start"
// thread(t1(0))   : makeThreads("t1 0", t2, 2)
// thread(t1(0))   : "t1 0 create 0"
// thread(t1(0))   : "t1 0 create 1"
// thread(t1(0))   : <join>
// thread(t1(1))   : "t1 1 start"
// thread(t1(1))   : makeThreads("t1 1", t2, 2)
// thread(t1(1))   : "t1 1 create 0"
// thread(t1(1))   : "t1 1 create 1"
// thread(t1(1))   : <join>
// thread(t2(0))   : "t2 0 start"
// thread(t2(0))   : <exit>
// thread(t2(1))   : "t2 1 start"
// thread(t2(1))   : <exit>
// thread(t2(0))   : "t2 0 start"
// thread(t2(0))   : <exit>
// thread(t2(1))   : "t2 1 start"
// thread(t2(1))   : <exit>
// thread(t1(1))   : <exit>
// thread(t1(1))   : "t1 1 end"
// thread(t1(0))   : <exit>
// thread(t1(0))   : "t1 0 end"
// thread(t0(NULL)): <exit>
// thread(t0(NULL)): "t0 end"
/////////////////////////////////////
// Mode: n=2 m=2 yield=1, join=1
// thread(t0(NULL)): "t0 start"
// thread(t0(NULL)): makeThreads("t0", t1, n=2)
// thread(t0(NULL)): "t0 create 0"
// thread(t0(NULL)): <yield>
// thread(t1(0))   : "t1 0 start"
// thread(t1(0))   : makeThreads("t1 0", t2, 2)
// thread(t1(0))   : "t1 0 create 0"







// thread(t0(NULL)): "t0 create 1"
// thread(t0(NULL)): <join>
// thread(t1(0))   : "t1 0 start"
// thread(t1(0))   : makeThreads("t1 0", t2, 2)
// thread(t1(0))   : "t1 0 create 0"
// thread(t1(0))   : "t1 0 create 1"
// thread(t1(0))   : <join>
// thread(t1(1))   : "t1 1 start"
// thread(t1(1))   : makeThreads("t1 1", t2, 2)
// thread(t1(1))   : "t1 1 create 0"
// thread(t1(1))   : "t1 1 create 1"
// thread(t1(1))   : <join>
// thread(t2(0))   : "t2 0 start"
// thread(t2(0))   : <exit>
// thread(t2(1))   : "t2 1 start"
// thread(t2(1))   : <exit>
// thread(t2(0))   : "t2 0 start"
// thread(t2(0))   : <exit>
// thread(t2(1))   : "t2 1 start"
// thread(t2(1))   : <exit>
// thread(t1(1))   : <exit>
// thread(t1(1))   : "t1 1 end"
// thread(t1(0))   : <exit>
// thread(t1(0))   : "t1 0 end"
// thread(t0(NULL)): <exit>
// thread(t0(NULL)): "t0 end"


