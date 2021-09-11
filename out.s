.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	movq $4, %r10
	movl %r10d, -8(%rbp)
	movl %r10d, -4(%rbp)
	movl -4(%rbp), %r11d
	movl -8(%rbp), %r12d
	sub %r12, %r11
	movl %r11d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
