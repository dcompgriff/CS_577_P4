#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

int thread_create(void (*fn)(void*), void* arg){
  void* stackPage = malloc(4096);
  return asm_create_thread(fn, arg, stackPage);
}

/*
Bonus Idea:
1) Call malloc still.
2) Add fn and arg to the allocated stackPage.
3) In clone, get fn and arg from ustack at the first and second 4 bytes.
4) Perform the rest of clone as usual.

Trap based approach. (Make 2 system calls).
1) Call malloc still.
2) Call clone. (But keep the process in the SLEEP state so that it doesn't get scheduled.
3) Call start_proc(uint pid, void (*fn)(void*), void* arg) 
that finds the proc, sets it's p->tf->eip, initializes its stack properly, and sets it 
to the runnable state.
*/

int thread_join(void){
  void* stackPage;
  int retPid = join(&stackPage);
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


void mutex_init(struct mutex* mtx){
  mtx->flag = 0;
  mtx->guard = (struct spinlock*)malloc(sizeof(struct spinlock));
  spin_init(mtx->guard);
  mtx->q = (pidQueue*)malloc(sizeof(pidQueue));
  queue_init(mtx->q);
}

/************************************************
MUTEX CODE.
************************************************/
void mutex_lock(struct mutex* mtx){
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

void mutex_unlock(struct mutex* mtx){
  spin_lock(mtx->guard);
  if(queue_empty(mtx->q)){
    // No more parked threads, so release the mutex flag lock.;
    mtx->flag = 0;
  }else{
    unpark(queue_remove(mtx->q));
  }
  spin_unlock(mtx->guard);
}

/************************************************
CONDITION VARIABLE CODE.
************************************************/
void cv_init(struct condvar* cv){
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
void cv_wait(struct condvar* cv, struct mutex* mtx){
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

/* wake one waiting thread */
void cv_signal(struct condvar* cv){
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

/* wake all waiting threads */
void cv_broadcast(struct condvar* cv){
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

/************************************************
SEMAPHORE CODE.
************************************************/
void sem_init(struct semaphore* sem, int initval){
  sem->cond = (struct condvar*)malloc(sizeof(struct condvar));
  cv_init(sem->cond);
  sem->mtx = (struct mutex*)malloc(sizeof(struct mutex));
  mutex_init(sem->mtx);
  sem->value = initval;
}

void sem_post(struct semaphore* sem){
  mutex_lock(sem->mtx);
  sem->value++;
  cv_signal(sem->cond);
  mutex_unlock(sem->mtx);
}

void sem_wait(struct semaphore* sem){
  mutex_lock(sem->mtx);
  while(sem->value < 0){
    cv_wait(sem->cond, sem->mtx);
  }
  sem->value--;
  mutex_unlock(sem->mtx);
}

/************************************************
pidQueue CODE.
************************************************/
void queue_init(pidQueue* q){
  q->head = NULL;
  q->tail = NULL;
  q->clock = (struct spinlock*)malloc(sizeof(struct spinlock));
  spin_init(q->clock);
}

uint queue_remove(pidQueue* q){
  uint pid = -1;
  if(q->head == NULL){
    return -1;
  }

  pidStruct* rem = q->head;
  if(q->head == q->tail){
    // List is now empty, set tail to NULL.
    q->head = NULL;
    q->tail = NULL;
  }else{
    // Move head pointer forward.
    q->head = q->head->next;
  }

  // Get the pid, and free the struct.
  pid = rem->pid;
  free(rem);
  return pid;
}

void queue_add(pidQueue* q, uint pid){
  pidStruct* new = (pidStruct*)malloc(sizeof(pidStruct));
  new->pid = pid;

  if(q->head == NULL && q->tail == NULL){
    // Empty list.
    q->head = new;
    q->tail = new;
    new->next = NULL;
  }else{
    q->tail->next = new;
    q->tail = new;
    new->next = NULL;
  }
}

int queue_empty(pidQueue* q){
  if(q->head == NULL && q->tail == NULL){
    return 1;
  }else{
    return 0;
  }
}






















