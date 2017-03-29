#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

int thread_create(void (*fn)(void*), void* arg){
  //Use 4000 so that enough space exists for the Header, and so only 1 page of mem loads.
  //Issues were found to occur when malloc(4096) was used.
  void* stackPage = malloc(4000);
  return clone(fn, arg, stackPage);
}

int thread_join(void){
  void* stackPage;
  int retPid = join(&stackPage);
  printf(1, "Freeing stack page allocated at %d.\n", stackPage);
  free(stackPage);
  return retPid;
}

void spin_init(struct spinlock* lk){
  lk->locked = 0;
}

void spin_lock(struct spinlock* lk){
  // The xchg is atomic.
  // It also serializes, so that reads after acquire are not
  // reordered before it. 
  while(xchg(&lk->locked, 1) != 0);
}

void spin_unlock(struct spinlock* lk){
  // The xchg serializes, so that reads before release are 
  // not reordered after it.  The 1996 PentiumPro manual (Volume 3,
  // 7.2) says reads can be carried out speculatively and in
  // any order, which implies we need to serialize here.
  // But the 2007 Intel 64 Architecture Memory Ordering White
  // Paper says that Intel 64 and IA-32 will not move a load
  // after a store. So lock->locked = 0 would work here.
  // The xchg being asm volatile ensures gcc emits it after
  // the above assignments (and after the critical section).
  xchg(&lk->locked, 0);
}












