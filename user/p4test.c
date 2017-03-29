#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX 5

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

void mutex_test(){
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

}

int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
struct condvar empty, fill;
struct mutex mtx;

void put(int value){
  buffer[fill_ptr] = value;
  fill_ptr = (fill_ptr + 1) % MAX;
  count++;
}

int get(){
  int tmp = buffer[use_ptr];
  use_ptr = (use_ptr + 1) % MAX;
  count--;
  return tmp;
}

void cond_producer(void* arg){
  int i;
  int loops = *(int*)arg;
  printf(1, "Prod loops %d.\n", loops);
  for(i = 0; i < loops; i++){
    //printf(1, "In producer.\n");
    mutex_lock(&mtx);
    while(count == MAX){
      //printf(1, "In producer, calling wait\n");
      cv_wait(&empty, &mtx);
    }
    //printf(1, "In producer, performing put(i).\n");
    put(i);
    //printf(1, "In producer, performing signal.\n");
    cv_signal(&fill);
    //printf(1, "In producer, unlocking.\n");
    mutex_unlock(&mtx);
  }
  exit();
}

void cond_consumer(void* arg){
  int i;
  int loops = *(int*)arg;
  printf(1, "Consumer loops %d.\n", loops);
  for(i = 0; i < loops; i++){
    //printf(1, "In consumer.\n");
    mutex_lock(&mtx);
    while(count == 0){
      //printf(1, "In consumer, calling wait\n");
      cv_wait(&fill, &mtx);
    }
    //printf(1, "In consumer, performing get(i).\n");
    int tmp = get();
    //printf(1, "In consumer, performing signal.\n");
    cv_signal(&empty);
    //printf(1, "In consumer, unlocking.\n");
    mutex_unlock(&mtx);
    printf(1, "%d\n", tmp);
  }
  exit();
}

void condition_variable_test(){
  cv_init(&empty);
  cv_init(&fill);
  mutex_init(&mtx);
  int final = 200;
  int loop1 = 100;
  
  uint t1Pid = thread_create(&cond_producer, (void*)&final);
  printf(1, "Producer thread %d created.\n", t1Pid);
  uint t2Pid = thread_create(&cond_consumer, (void*)&loop1);
  printf(1, "Consumer thread %d created.\n", t2Pid);
  uint t3Pid = thread_create(&cond_consumer, (void*)&loop1);
  printf(1, "Consumer thread %d created.\n", t3Pid);

  /*
  uint res1 = thread_join();
  printf(1, "TPid %d finished.\n", res1);
  uint res2 = thread_join();
  printf(1, "TPid %d finished.\n", res2);
  uint res3 = thread_join();
  printf(1, "TPid %d finished.\n", res3);
  */
  thread_join();
  thread_join();
  thread_join();

}

int
main(int argc, char *argv[])
{
  mutex_test();
  condition_variable_test();

  exit();
}





















