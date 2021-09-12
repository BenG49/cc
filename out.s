.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $12, %rsp
	movq $0, %r10
	movl %r10d, -4(%rbp)
	movq $2, %r10
	movl %r10d, -8(%rbp)
	movq $1, %r10
	movl %r10d, -12(%rbp)
	movl -12(%rbp), %r10d
	movl %r10d, -8(%rbp)
	add $4, %rsp
	movl -8(%rbp), %r10d
	movl %r10d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
