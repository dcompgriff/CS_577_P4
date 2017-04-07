#ifndef __PID_QUEUE__
#define __PID_QUEUE__
#include "types.h"

/*
PID QUEUE STRUCTS.
*/
// Struct to hold a pid for the pidQueue.
typedef struct __pidStruct {
  uint pid;
  struct mutex* pidMutex;
  struct __pidStruct* next;
}pidStruct;

// Queue struct to hold the pids.
typedef struct __pidQueue {
  pidStruct* head;
  pidStruct* tail;
  struct spinlock* clock;
}pidQueue;

void queue_init(pidQueue* q);
uint queue_remove(pidQueue* q);
void queue_add(pidQueue*q, uint pid);
int queue_empty(pidQueue* q);

#include "mutex.h"

#endif



