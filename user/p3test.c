#include "types.h"
#include "stat.h"
#include "user.h"

void
run0(void* arg){
  printf(1, "In thread.\n");
  exit();
}

void
run(void* arg){
    printf(1, "In thread1. Calling park.\n");
    setpark();
    park();
    printf(1, "In thread1. Unpark called!\n");
    exit();
}

void
runf1(void* arg){
    printf(1, "In thread2. Calling setpark.\n");
    int ret = setpark();
    printf(1, "In thread2. First setpark was %d.\n", ret);
    printf(1, "In thread2. Calling setpark again.\n");
    ret = setpark();
    printf(1, "In thread2. Second setpark was %d. Should be -1.\n", ret);
    park();
    printf(1, "In thread2. Unpark called!\n");
    exit();
}

void
runf2(void* arg){
    printf(1, "In thread3. Calling sleep(700).\n");
    sleep(500);
    printf(1, "In thread3. Awoke!\n");
    printf(1, "In thread3. Calling setpark.\n");
    setpark();
    printf(1, "In thread3. Calling park.\n");
    park();
    printf(1, "In thread3. Unpark called!\n");
    exit();
}

int
main(int argc, char *argv[])
{
  //First test.
  uint t1Pid = thread_create(&run, NULL);
  printf(1, "Parent Sleeping.\n");
  sleep(700);
  printf(1, "Parent Awoke! Calling unpark...\n");
  int unparkRet = unpark(t1Pid);
  printf(1, "Unpark Called! Waiting on thread...\n");
  thread_join();
  printf(1, "Parent finished. unparkRet = %d.\n", unparkRet);

  //Second test.
  t1Pid = thread_create(&runf1, NULL);
  printf(1, "Parent Sleeping.\n");
  sleep(700);
  printf(1, "Parent Awoke! Calling unpark...\n");
  unparkRet = unpark(t1Pid);
  printf(1, "Unpark Called! Waiting on thread...\n");
  thread_join();
  printf(1, "Parent finished. unparkRet = %d.\n", unparkRet);

  //Third test.
  t1Pid = thread_create(&runf2, NULL);
  printf(1, "Parent calling unpark before child's park call.\n");
  unparkRet = unpark(t1Pid);
  printf(1, "Unpark call returned %d. Should be -1. Parent Sleeping...\n", unparkRet);
  sleep(700);
  printf(1, "Parent Awoke! Calling unpark...\n");
  unparkRet = unpark(t1Pid);
  printf(1, "Unpark Called! Waiting on thread...\n");
  thread_join();
  printf(1, "Parent finished. unparkRet = %d.\n", unparkRet);


  exit();
}
