.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	
	mov $10, %eax
	mov %eax, -4(%rbp)
	movl -4(%rbp), %eax
	push %rax
	inc %eax
	mov %eax, -4(%rbp)
	pop %rax
	push %rax
	mov $2, %eax
	pop %rcx
	add %ecx, %eax
	mov %eax, -8(%rbp)
	movl -8(%rbp), %eax

	mov %rbp, %rsp
	pop %rbp
	ret
	
	xor %eax, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
