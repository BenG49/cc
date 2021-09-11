.globl main
main:
	push %rbp
	mov %rsp, %rbp
	movq $12, %r10
	neg %r10
	movq $5, %r11
	movq %r10, %rax
	cqo
	idiv %r11
	movq %rax, %r11
	movl %r11d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
