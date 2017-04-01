#include "types.h"
#include "stat.h"
#include "user.h"

void run(void* arg);

int
main(int argc, char *argv[])
{

    int i = 33;
    int pid = thread_create(&run, (void*)&i);
    printf(1, "Pid Status was %d.\n", pid);
    int retVal = thread_join();
    printf(1, "Thread returned! Pid was %d.\n", retVal);
    exit();

/*
    int i = 10;
    void* page1 = malloc(4096);
    int pid = clone(&run, (void*)&i, page1);
    printf(1, "Pid Status was %d.\n", pid);

    i = 20;
    void* page2 = malloc(4096);
    pid = clone(&run, (void*)&i, page2);
    printf(1, "Pid Status was %d.\n", pid);

    void* retPage;
    int waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page1 = %d; page2 = %d; retPage = %d.\n", page1, page2, retPage);
    free(retPage);

    waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page1 = %d; page2 = %d; retPage = %d.\n", page1, page2, retPage);
    free(retPage);


    i = 30;
    printf(1, "Before third malloc.\n");
    page1 = malloc(4096);
    printf(1, "After third malloc.\n");
    pid = clone(&run, (void*)&i, page1);
    printf(1, "Pid Status was %d.\n", pid);

    i = 40;
    page2 = malloc(4096);
    pid = clone(&run, (void*)&i, page2);
    printf(1, "Pid Status was %d.\n", pid);

    waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page1 = %d; page2 = %d; retPage = %d.\n", page1, page2, retPage);
    free(retPage);

    waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page1 = %d; page2 = %d; retPage = %d.\n", page1, page2, retPage);
    free(retPage);
*/

/*
    i = 10;
    page = malloc(4096);
    pid = clone(&run, (void*)&i, page);
    printf(1, "Pid Status was %d.\n", pid);

    waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page = %d; retPage = %d.\n", page, retPage);
    free(retPage);

    i = 10;
    page = malloc(4096);
    pid = clone(&run, (void*)&i, page);
    printf(1, "Pid Status was %d.\n", pid);

    waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page = %d; retPage = %d.\n", page, retPage);
    free(retPage);

    i = 10;
    page = malloc(4096);
    pid = clone(&run, (void*)&i, page);
    printf(1, "Pid Status was %d.\n", pid);

    waitPid = join(&retPage);
    printf(1, "Parent waitPid is: %d.\n", waitPid);
    printf(1, "page = %d; retPage = %d.\n", page, retPage);
    free(retPage);
*/
}





void
run(void* arg){
    printf(1, "In thread, passed %d.\n", *(int*)arg);
    exit();
}







