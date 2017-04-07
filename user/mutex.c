
#include "types.h"
#include "user.h"

void mutex_init(struct mutex* mtx)
{
  mtx->flag = 0;
  mtx->guard = (struct spinlock*)malloc(sizeof(struct spinlock));
  spin_init(mtx->guard);
  mtx->q = (pidQueue*)malloc(sizeof(pidQueue));
  queue_init(mtx->q);
}

void mutex_lock(struct mutex* mtx)
{
  // Aquire guard lock.
  spin_lock(mtx->guard);

  if(mtx->flag == 0){
    // Set flag to indicate we are taking the open lock.
    mtx->flag = 1;
    spin_unlock(mtx->guard);
  }else{
    // We can't get the flag lock right now, so sleep until awoken.
    // When awoken, we will implicitly have the lock.
    queue_add(mtx->q, getpid());
    setpark();
    spin_unlock(mtx->guard);
    park();
  }
}

void mutex_unlock(struct mutex* mtx)
{
  spin_lock(mtx->guard);
  if(queue_empty(mtx->q)){
    // No more parked threads, so release the mutex flag lock.;
    mtx->flag = 0;
  }else{
    unpark(queue_remove(mtx->q));
  }
  spin_unlock(mtx->guard);
}

