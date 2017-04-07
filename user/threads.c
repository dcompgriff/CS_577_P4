
#include "types.h"
#include "user.h"

int thread_create(void (*thfunc)(void*), void* arg)
{
  void* stackPage = malloc(4096);
  return asm_create_thread(thfunc, arg, stackPage);
}

int thread_join(void)
{
  void* stackPage;
  int retPid = join(&stackPage);
  free(stackPage);
  return retPid;
}
