
#ifndef CONDVAR_H
#define CONDVAR_H
#include "pidqueue.h"
#include "mutex.h"

// Condition signal structure.
struct condvar {
  pidQueue* q;
};

void cv_init(struct condvar* cv);
void cv_wait(struct condvar* cv, struct mutex* mtx);
void cv_signal(struct condvar* cv);
void cv_broadcast(struct condvar* cv);

#endif
