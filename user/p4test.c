#include "types.h"
#include "stat.h"
#include "user.h"

void
run0(void* arg){
  printf(1, "In thread.\n");
  exit();
}

struct additionStruct {
  int totalCount;
  int taCount;
  int tbCount;
  struct mutex* mtx;
}mStruct;

void
add(void* arg){
  while(mStruct.totalCount < 100000){
    mutex_lock(mStruct.mtx);
    if(mStruct.totalCount >= 100000){
      mutex_unlock(mStruct.mtx);
      continue;
    }
    mStruct.totalCount += 1;
    if(*(int*)arg == 0){
      mStruct.taCount++;
    }else{
      mStruct.tbCount++;
    }
    mutex_unlock(mStruct.mtx);  
  }
  exit();
}


void
add2(void* arg){
  int count = 0;
  while(mStruct.totalCount < 500000){
    count = 0;
    mutex_lock(mStruct.mtx);
    if(mStruct.totalCount >= 500000){
      mutex_unlock(mStruct.mtx);
      continue;
    }

    while(mStruct.totalCount < 500000 && count < 1000){
      mStruct.totalCount += 1;
      count++;
      if(*(int*)arg == 0){
        mStruct.taCount++;
      }else{
        mStruct.tbCount++;
      }
    }
    mutex_unlock(mStruct.mtx); 
  }
  exit();
}

int
main(int argc, char *argv[])
{
  // Shared variable access test.
  int ta = 1;
  int tb = 0;
  mStruct.totalCount = 0;
  mStruct.taCount = 0;
  mStruct.tbCount = 0;
  mStruct.mtx = (struct mutex*)malloc(sizeof(struct mutex));
  mutex_init(mStruct.mtx);

  //This ordering causes the second thread to finish first. It tests
  //if the system is ok with freeing the stack page malloc'd at a lower address before the 
  //higher address.
  printf(1, "Creating first thread.\n");
  uint t1Pid = thread_create(&add, (void*)&ta);
  printf(1, "Creating second thread.\n");
  uint t2Pid = thread_create(&add, (void*)&tb);

  printf(1, "Calling third join.\n");
  uint res1 = thread_join();
  printf(1, "T%d finished.\n", res1);
  printf(1, "Calling fourth join.\n");
  uint res2 = thread_join();
  printf(1, "T%d finished.\n", res2);

  printf(1, "Final addition count is: %d.\n", mStruct.totalCount);
  printf(1, "Ta, pid %d, count: %d.\n", t1Pid, mStruct.taCount);
  printf(1, "Tb, pid %d, count: %d.\n", t2Pid, mStruct.tbCount);

  //Run the same adding threads, but in a symmetric fashion now.

  mStruct.totalCount = 0;
  mStruct.taCount = 0;
  mStruct.tbCount = 0;
  ta = 1;
  tb = 0;
  printf(1, "Creating third thread.\n");
  t1Pid = thread_create(&add2, (void*)&ta);
  printf(1, "Creating fourth thread.\n");
  t2Pid = thread_create(&add2, (void*)&tb);

  printf(1, "Calling first join.\n");
  res1 = thread_join();
  printf(1, "T%d finished.\n", res1);
  printf(1, "Calling second join.\n");
  res2 = thread_join();
  printf(1, "T%d finished.\n", res2);

  printf(1, "Final addition count is: %d.\n", mStruct.totalCount);
  printf(1, "Ta, pid %d, count: %d.\n", t1Pid, mStruct.taCount);
  printf(1, "Tb, pid %d, count: %d.\n", t2Pid, mStruct.tbCount);

  exit();
}
