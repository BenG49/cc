.globl main
main:
	push %rbp
	mov %rsp, %rbp
	movq $1, %r10
	test %r10, %r10
	je L0
	movq $1, %r10
	jmp L1
L0:
	movq $0, %r11
	test %r11, %r11
	mov $0, %r10
	setne %r10b
L1:
	movl %r10d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
