
#include "types.h"
#include "user.h"
#include "pidqueue.h"
#include "condvar.h"
#include "mutex.h"

void sem_init(struct semaphore* sem, int initval)
{
  sem->cond = (struct condvar*)malloc(sizeof(struct condvar));
  cv_init(sem->cond);
  sem->mtx = (struct mutex*)malloc(sizeof(struct mutex));
  mutex_init(sem->mtx);
  sem->value = initval;
}

void sem_wait(struct semaphore* sem)
{
  mutex_lock(sem->mtx);
  while(sem->value <= 0){
    cv_wait(sem->cond, sem->mtx);
  }
  sem->value--;
  mutex_unlock(sem->mtx);
}

void sem_post(struct semaphore* sem)
{
  mutex_lock(sem->mtx);
  sem->value++;
  cv_signal(sem->cond);
  mutex_unlock(sem->mtx);
}

