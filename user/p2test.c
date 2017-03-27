#include "types.h"
#include "stat.h"
#include "user.h"

struct additionStruct {
  int totalCount;
  int taCount;
  int tbCount;
  struct spinlock* mLock;
}mStruct;

void
add(void* arg){
  while(mStruct.totalCount < 100000){
    spin_lock(mStruct.mLock);
    if(mStruct.totalCount >= 100000){
      break;
    }
    mStruct.totalCount += 1;
    if(*(int*)arg == 0){
      mStruct.taCount++;
    }else{
      mStruct.tbCount++;
    }
    spin_unlock(mStruct.mLock);  
  }
  exit();
}

void
run(void* arg){
    printf(1, "In thread, passed %d.\n", *(int*)arg);
    exit();
}


int
main(int argc, char *argv[])
{
  int ta = 1;
  int tb = 0;
  mStruct.totalCount = 0;
  mStruct.taCount = 0;
  mStruct.tbCount = 0;
  mStruct.mLock = (struct spinlock*)malloc(sizeof(struct spinlock));

  spin_init(mStruct.mLock);
  //uint t1Pid = thread_create(&run, (void*)1);
  //uint t2Pid = thread_create(&run, (void*)0);
  uint t1Pid = thread_create(&add, (void*)&ta);
  uint t2Pid = thread_create(&add, (void*)&tb);

  uint res1 = thread_join();
  printf(1, "T%d finished.\n", res1);
  uint res2 = thread_join();
  printf(1, "T%d finished.\n", res2);

  printf(1, "Final addition count is: %d.\n", mStruct.totalCount);
  printf(1, "Ta, pid %d, count: %d.\n", t1Pid, mStruct.taCount);
  printf(1, "Tb, pid %d, count: %d.\n", t2Pid, mStruct.tbCount);

  exit();
}
