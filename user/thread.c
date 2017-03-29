#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

int thread_create(void (*fn)(void*), void* arg){
  void* stackPage = malloc(4096);
  return clone(fn, arg, stackPage);
}

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
  printf(1, "Mutex Lock Called! Current lock state is %d.\n", mtx->guard->locked);
  spin_lock(mtx->guard);

  if(mtx->flag == 0){
    // Set flag to indicate we are taking the open lock.
    mtx->flag = 1;
    spin_unlock(mtx->guard);
    printf(1, "mtx flag is 0 (mutex is not locked), so thread locked it and continued.\n");
  }else{
    // We can't get the flag lock right now, so sleep until awoken.
    // When awoken, we will implicitly have the lock.
    printf(1, "Thread can't get lock, so add itself to queue and park.\n");
    queue_add(mtx->q, getpid());
    printf(1, "Thread added itself to the queue. Calling setpark and unlocking.\n");
    setpark();
    spin_unlock(mtx->guard);
    printf(1, "Thread calling park.\n");
    park();
    printf(1, "Thread woken up after park! Continuing execution.\n");
  }
}

void mutex_unlock(struct mutex* mtx){
  printf(1, "Mutex Unlock Called! Current lock state is %d.\n", mtx->guard->locked);
  spin_lock(mtx->guard);
  if(queue_empty(mtx->q)){
    // No more parked threads, so release the mutex flag lock.
    printf(1, "No more parked threads, so setting flag to 0.\n");
    mtx->flag = 0;
  }else{
    printf(1, "Calling unpark for thread %d.\n", mtx->q->head->pid);
    unpark(queue_remove(mtx->q));
  }
  spin_unlock(mtx->guard);
}

/************************************************
CONDITION VARIABLE CODE.
************************************************/
void cv_init(struct condvar* cv){

}

void cv_wait(struct condvar* cv, struct mutex* mtx){

}

/* wake one waiting thread */
void cv_signal(struct condvar* cv){

}

/* wake all waiting threads */
void cv_broadcast(struct condvar* cv){

}

/************************************************
SEMAPHORE CODE.
************************************************/
void sem_init(struct semaphore* sem, int initval){

}

void sem_post(struct semaphore* sem){

}

void sem_wait(struct semaphore* sem){

}

/************************************************
pidQueue CODE.
************************************************/
void queue_init(pidQueue* q){
  q->head = NULL;
  q->tail = NULL;
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






















