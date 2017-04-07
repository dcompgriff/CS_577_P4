
.section .text
.globl asm_create_thread

.type asm_create_thread, @function
asm_create_thread:
	#Initialize the stack.
	pushl %ebp     				#Save the old base pointer value.
	movl  %esp, %ebp 			#Set the new base pointer value.
	#Store callee saved registers that will be used.
	pushl %edi
	pushl %esi
	pushl %ebx

	#Store void (*)(void*) pointer, and void* in 2 regs.
	movl 16(%ebp), %ebx			#void* ustack (Last argument)
	movl 12(%ebp), %edi			#void* address (Middle argument)
	movl 8(%ebp), %esi			#void (*)(void*) address (First argument)
	
	#Push malloc argument onto the stack for the trap.
	pushl 16(%ebp)				#(push void* ustack)
	pushl Comp				#Push return address.


	#Call trap handler
	movl $22, %eax              		#Move trap #22 (For sys_clone)
	int  $64                   		#Call the system interupt command using 'int', with '64'
Comp:
	cmpl  $0, %eax
	#test %eax, %eax				#If 0 returned, then this is the thread, so jump to the thread execution label
	je   ThreadRet
	addl $8, %esp				#Pop the malloc  and ustack argument off of the stack after the interrupt returned.	(or use addl $4, %esp )
	popl %ebx				#Restore the callee saved registers values.
	popl %esi				
	popl %edi
	movl %ebp, %esp 			#Deallocate local variables
	popl %ebp 				#Restore the caller's base pointer value
	ret					#This is the parent, so simply return.
				   		

ThreadRet:
	addl  $4096, %ebx
	movl  %ebx, %ebp
	movl  %ebx, %esp
	pushl %edi				#Push void* arg onto stack.
	pushl $0xffffffff			#Push dummy return address onto stack.
	jmp   %esi

