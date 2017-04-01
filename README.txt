Author: Daniel Griffin
Date: 3/31/17

************************************************
OS Project 4: Implementing Concurrency in xv6
************************************************
Project Description: 
This project consisted of 4 major parts.

************************************************
Part 1
************************************************
Part 1 Description of Implementation:


************************************************
Part 2
************************************************
Part 2 Description of Implementation:


************************************************
Part 3
************************************************
Part 3 Description of Implementation:


************************************************
Part 4
************************************************
Part 4 Description of Implementation:



************************************************
Bonus
************************************************
**As an aside, this bonus took me 2 full days to complete.
I had to:
1) Learn x86 assembly.
2) Learn the GCC C calling conventions.
3) Learn the conventions for the int command, as well as what the xv6 setup expected when it was called (Expects a return address before the 'int' instruction).
3) Learn how to incorporate mclone.S into the build.
4) Learn (the hard way) that there are multiple syntaxes for x86 asm.
5) Learn how to use gdb with assembly language code (disas, reg/cpu flag examine, etc...)
6) Learn how to debug the x86 assembly code for the user space code (Before I found out how to load user space symbol tables, I'd set break points in the sys_clone function and step through the assembly 'ret' commands to work my way back to user space code).
7) Learn how xv6 worked with the fork command and allocproc() to set up the new process.

Bonus Implementation Description:
The overall breakdown of program flow is this:
1) thread_create() is called by a user program.
2) thread_create creates a stack page with malloc(), then calls a function implemented in assembly named "asm_create_thread(void (*fn)(void*), void* arg, void* ustack);
3) The assembly function stores the arguments passed to it in 3 registers edi, esi, ebx (The code saves the original registers (as they are 'callee saved') before modifying the registers).
4) The void* ustack argument is pushed onto the stack, and then the sys_clone trap instruction is called.
5) sys_clone gets void *ustack and calls clone(). Clone sets up a new process (essentially the same as fork), but with references to the same address space. (Note that copies of ebx, edi, and esi are made here).
6) clone then returns, and then sys_clone returns.
7) The trap code then returns to the mclone.S assembly code label 'Comp' (for comparison).
8) The returned value of eax is tested.
9) If it is not 0, then the process is a parent, and the state of the parent thread (including original ebx, esi, and edi regs) is restored, and the parent calls ret.
10) If the returned eax value is 0, then the code jumps to the ThreadRet label.
11) Here, it adds 4096 to ebx (which still holds void* ustack), sets the ebp and esp regs to this new address, pushes the argument onto the stack, pushes the dummy return function 0xffffffff onto the stack, then jumps to the address of the function (still stored in esi).

Thus, the parent returns to running its original code, while the child continues on to run the function that was passed to it. The entire process hinges on the fact that clone makes copies of edi, esi, and ebx, which hold all of the information we need to continue execution of the thread when it returns.


