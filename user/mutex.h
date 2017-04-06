#ifndef MUTEX_H
#define MUTEX_H
#include "pidqueue.h"
#include "spinlock.h"


// Mutual exclusion blocking lock.
struct mutex {
  int flag;
  struct spinlock* guard;
  pidQueue* q;
};

void mutex_init(struct mutex* mtx);
void mutex_lock(struct mutex* mtx);
void mutex_unlock(struct mutex* mtx);


#endif
