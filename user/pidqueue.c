#include "user.h"

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


















