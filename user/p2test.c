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
  while(mStruct.totalCount < 1000000){
    spin_lock(mStruct.mLock);
    if(mStruct.totalCount >= 1000000){
      spin_unlock(mStruct.mLock);
      continue;
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
add2(void* arg){
  while(mStruct.totalCount < 1000000){
    spin_lock(mStruct.mLock);
    if(mStruct.totalCount >= 1000000){
      spin_unlock(mStruct.mLock);
      continue;
    }
    mStruct.totalCount += 1;
    if(*(int*)arg == 0){
      mStruct.taCount++;
    }else{
      mStruct.tbCount++;
    }
    spin_unlock(mStruct.mLock);
    sleep(10);  
  }
  exit();
}

void
run(void* arg){
    printf(1, "In thread, passed %d.\n", *(int*)arg);
    exit();
}

void
run0(void* arg){
  printf(1, "In thread.\n");
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
  mStruct.mLock = (struct spinlock*)malloc(sizeof(struct spinlock));

  spin_init(mStruct.mLock);
  //This ordering causes the second thread to finish first. It tests
  //if the system is ok with freeing the stack page malloc'd at a lower address before the 
  //higher address.
  uint t1Pid = thread_create(&add2, (void*)&ta);
  uint t2Pid = thread_create(&add, (void*)&tb);

  uint res1 = thread_join();
  printf(1, "T%d finished.\n", res1);
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
  spin_init(mStruct.mLock);
  t1Pid = thread_create(&add, (void*)&ta);
  t2Pid = thread_create(&add, (void*)&tb);

  res1 = thread_join();
  printf(1, "T%d finished.\n", res1);
  res2 = thread_join();
  printf(1, "T%d finished.\n", res2);

  printf(1, "Final addition count is: %d.\n", mStruct.totalCount);
  printf(1, "Ta, pid %d, count: %d.\n", t1Pid, mStruct.taCount);
  printf(1, "Tb, pid %d, count: %d.\n", t2Pid, mStruct.tbCount);

 
  // Consecutive threads test.
  printf(1, "Creating t1.\n");
  thread_create(&run0, NULL);
  printf(1, "Created t1, calling join.\n");
  thread_join();
  printf(1, "Join for t1 returned, Creating t2.\n");
  thread_create(&run0, NULL);
  printf(1, "Created t2, calling join.\n");
  thread_join();
  printf(1, "Join for t2 returned, finished!\n");

  printf(1, "Creating t1 AND t2.\n");
  thread_create(&run0, NULL);
  thread_create(&run0, NULL);
  printf(1, "Created t1 AND t2, calling join for both.\n");
  thread_join();
  thread_join();
  printf(1, "Join for t1 AND t2 returned.\n");

  printf(1, "Creating t1 AND t2.\n");
  thread_create(&run0, NULL);
  thread_create(&run0, NULL);
  printf(1, "Created t1 AND t2, calling join for both.\n");
  thread_join();
  thread_join();
  printf(1, "Join for t1 AND t2 returned.\n");
  

  exit();
}
