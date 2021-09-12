.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	movq $0, %r10
	movl %r10d, -4(%rbp)
	movq $0, %r10
	movl %r10d, -8(%rbp)
L1:
	movl -8(%rbp), %r11d
	movq $10, %r12
	cmp %r12, %r11
	jg L3
	movl -8(%rbp), %r10d
	movq $5, %r11
	cmp %r11, %r10
	jne L4
	jmp L3
L4:
	movl -8(%rbp), %r10d
	movl -4(%rbp), %r11d
	add %r11, %r10
	movl %r10d, -4(%rbp)
L2:
	movl -8(%rbp), %r10d
	inc %r10
	movl %r10d, -8(%rbp)
	dec %r10
	jmp L1
L3:
	add $4, %rsp
	movl -4(%rbp), %r10d
	movl %r10d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
	add $538976288, %rsp
