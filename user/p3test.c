#include "types.h"
#include "stat.h"
#include "user.h"

void
run(void* arg){
    printf(1, "In thread. Calling park.\n");
    setpark();
    park();
    printf(1, "In thread. Unpark called!\n");
    exit();
}

int
main(int argc, char *argv[])
{

  uint t1Pid = thread_create(&run, NULL);
  printf(1, "Parent Sleeping.\n");
  sleep(1000);
  printf(1, "Parent Awoke! Calling unpark...\n");
  int unparkRet = unpark(t1Pid);
  printf(1, "Unpark Called! Waiting on thread...\n");
  thread_join();
  printf(1, "Parent finished. unparkRet = %d.\n", unparkRet);

  exit();
}
