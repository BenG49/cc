.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $4, %rsp
	movq $0, %r10
	movl %r10d, -4(%rbp)
	movl -4(%rbp), %r11d
	movq $1, %r12
	neg %r12
	cmp %r12, %r11
	jle L0
	movq $4, %r10
L0:
	movq $5, %r10
L1:
	movl %r10d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
