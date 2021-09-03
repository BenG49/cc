.globl main
main:
	push %rbp
	mov %rsp, %rbp
	
	mov $1, %eax
	push %rax
	mov $1, %eax
	pop %rcx
	cmp %ecx, %eax
	mov $0, %eax
	sete %al
	test %eax, %eax
	jz uwu0
	mov $0, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
	jmp uwu1
uwu0:
	mov $1, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
uwu1:
	
	xor %eax, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
