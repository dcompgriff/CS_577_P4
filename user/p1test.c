#include "types.h"
#include "stat.h"
#include "user.h"

void
run(void* arg){
    printf(1, "In thread, passed %d.\n", *(int*)arg);
    exit();
}

int
main(int argc, char *argv[])
{
    int i = 10;
    void* page = malloc(4096);
    int pid = clone(&run, (void*)&i, page);
    printf(1, "Pid Status was %d.\n", pid);

    void* retPage;
    int waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page = %d; retPage = %d.\n", page, retPage);
    free(retPage);

    exit();
}
