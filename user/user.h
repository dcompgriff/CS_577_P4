#ifndef _USER_H_
#define _USER_H_

struct stat;

// Mutual exclusion spin lock.
struct spinlock {
  uint locked;            // Is locked.
};

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

// Mutual exclusion blocking lock.
struct mutex {
  int flag;
  struct spinlock* guard;
  pidQueue* q;
};

// Condition signal structure.
struct condvar {
  pidQueue* q;
};

// Semaphore structure.
struct semaphore {
  int value;
  struct condvar* cond;
  struct mutex* mtx;
};


// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int clone(void*);
int join(void**);
void park(void);
int setpark(void);
int unpark(int pid);

// Special asm code call for thread creation with clone.
int asm_create_thread(void (*fn)(void*), void* arg, void* ustack);

// user library functions (ulib.c)
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

// user library functions for threads.
int thread_create(void (*)(void*), void*);
int thread_join(void);
void spin_init(struct spinlock*);
void spin_lock(struct spinlock*);
void spin_unlock(struct spinlock*);
void mutex_init(struct mutex* mtx);
void mutex_lock(struct mutex* mtx);
void mutex_unlock(struct mutex* mtx);
void cv_init(struct condvar* cv);
void cv_wait(struct condvar* cv, struct mutex* mtx);
void cv_signal(struct condvar* cv); /* wake one waiting thread */
void cv_broadcast(struct condvar* cv); /* wake all waiting threads */
void sem_init(struct semaphore* sem, int initval);
void sem_post(struct semaphore* sem);
void sem_wait(struct semaphore* sem);
void queue_init(pidQueue* q);
uint queue_remove(pidQueue* q);
void queue_add(pidQueue*q, uint pid);
int queue_empty(pidQueue* q);

// user library thread queue functions.

#endif // _USER_H_

