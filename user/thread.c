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
  cv->q = (condQueue*)malloc(sizeof(condQueue));
  cond_queue_init(cv->q);
}

void cv_wait(struct condvar* cv, struct mutex* mtx){
  // Lock before modifying cond queue.
  spin_lock(cv->q->clock);
  cond_queue_add(cv->q, getpid(), mtx);
  // Release the passed lock.
  mutex_unlock(mtx);
  spin_unlock(cv->q->clock);


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



}

/* wake one waiting thread */
void cv_signal(struct condvar* cv){
  // Lock before modifying cond queue.
  spin_lock(cv->q->clock);
  if(!cond_queue_empty()){
    condStruct* ret = cond_queue_remove();
    spin_unlock(cv->q->clock);
    // Re-lock on the passed lock.
    mutex_lock(ret->mtx);
    // Free ret.
    free(ret);
  }else{
    //Queue empty, so simply release the lock.
    spin_unlock(cv->q->clock);
  }
}

/* wake all waiting threads */
void cv_broadcast(struct condvar* cv){
  condStruct* head = NULL;
  // Lock before modifying cond queue.
  if(!cond_queue_empty()){
    spin_lock(cv->q->clock);
    head = cv->q->head;
    cv->q->head = NULL;
    cv->q->tail = NULL;
    spin_unlock(cv->q->clock);
    while(head != NULL){
      ret = cond_queue_remove();
      curr->next = ret
    }

    condStruct* ret = cond_queue_remove();
    spin_unlock(cv->q->clock);
    // Re-lock on the passed lock.
    mutex_lock(ret->mtx);
  }else{
    //Queue empty, so simply release the lock.
    spin_unlock(cv->q->clock);
  }
}

void cond_queue_init(condQueue* q){
  q->head = NULL;
  q->tail = NULL;
  q->clock = (struct spinlock*)malloc(sizeof(struct spinlock));
  spin_init(q->clock);
}

// NOTE: Caller must call free on the returned struct.
condStruct* cond_queue_remove(condQueue* q){
  if(cond_queue_enpty(q)){
    return NULL;
  }

  condStruct* rem = q->head;
  if(q->head == q->tail){
    // List is now empty, set tail to NULL.
    q->head = NULL;
    q->tail = NULL;
  }else{
    // Move head pointer forward.
    q->head = q->head->next;
  }

  // Get the pid, and free the struct.
  return rem;
}

void cond_queue_add(condQueue*q, uint pid, struct mutex *mtx){
  condStruct* new = (condStruct*)malloc(sizeof(condStruct));
  new->pid = pid;
  new->mtx = mtx;

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

int cond_queue_empty(condQueue* q){
  if(q->head == NULL && q->tail == NULL){
    return 1;
  }else{
    return 0;
  }
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






















