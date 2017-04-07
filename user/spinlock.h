
#ifndef SPINLOCK_H
#define SPINLOCK_H
#include "types.h"

// Mutual exclusion spin lock.
struct spinlock {
  uint locked;            // Is locked.
};

void spin_init(struct spinlock* lk);
void spin_lock(struct spinlock *lk);
void spin_unlock(struct spinlock *lk);

#endif

