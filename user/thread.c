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













