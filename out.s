.globl main
main:
	push %rbp
	mov %rsp, %rbp
	movq $0, %r10
	test %r10, %r10
	je L1
	movq $1, %r10
	jmp L2
L1:
	movq $1, %r11
	test %r11, %r11
	mov $0, %r10
	setne %r10b
L2:
	movl %r10d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
