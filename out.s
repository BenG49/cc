.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $12, %rsp
	movl $0, %r10d
	movl %r10d, -4(%rbp)
	movl -4(%rbp), %r10d
%r10
	movl %r10d, -12(%rbp)
	movl -12(%rbp), %r10d
	movl %r10d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
