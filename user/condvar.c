
#include "types.h"
#include "user.h"
#include "pidqueue.h"

void cv_init(struct condvar* cv)
{
  cv->q = (pidQueue*)malloc(sizeof(pidQueue));
  queue_init(cv->q);
}

/*
1) Lock on cond queue.
2) Add process pid to queue.
3) Call setpart.
4) unlock mutex.
5) unlock cond queue.
6) park()
7) relock mutext.
8) return from function.
*/
void cv_wait(struct condvar* cv, struct mutex* mtx)
{
  // Lock before modifying cond queue.
  //printf(1, "Trying to aquire cv spinlock in wait.\n");
  spin_lock(cv->q->clock);
  //printf(1, "cv spinlock aquired.\n");
  queue_add(cv->q, getpid());
  setpark();
  // Release the locks.
  spin_unlock(cv->q->clock);
  mutex_unlock(mtx);

  // Park the process.
  park();

  // Once awoken from park, re-gain the lock.
  mutex_lock(mtx);
}

void cv_signal(struct condvar* cv)
{
  uint pid;
  // Lock before modifying cond queue.
  //printf(1, "Trying to aquire cv spinlock in signal.\n");
  spin_lock(cv->q->clock);
  //printf(1, "cv spinlock aquired.\n");
  if(!queue_empty(cv->q)){
    pid = queue_remove(cv->q);
    unpark(pid);
  }
  spin_unlock(cv->q->clock);
}

void cv_broadcast(struct condvar* cv)
{
  uint pid;
  // Lock before modifying cond queue.
  spin_lock(cv->q->clock);
  while(!queue_empty(cv->q)){
    pid = queue_remove(cv->q);
    unpark(pid);
  }
  //Queue empty, so simply release the lock.
  spin_unlock(cv->q->clock);
}




