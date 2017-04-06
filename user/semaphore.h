
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

// Semaphore structure.
struct semaphore {
  int value;
  struct condvar* cond;
  struct mutex* mtx;
};

void sem_init(struct semaphore* sem, int initval);
void sem_post(struct semaphore* sem);
void sem_wait(struct semaphore* sem);

#endif
