.globl main
main:
	push %rbp
	mov %rsp, %rbp
	
	mov $1, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
	
	xor %eax, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
