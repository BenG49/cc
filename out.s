.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $4, %rsp
	
	mov $3, %eax
	mov %eax, -4(%rbp)
	mov $3, %eax
	push %rax
	movl -4(%rbp), %eax
	pop %rcx
	cmp %eax, %ecx
	mov $0, %eax
	setne %al
	test %eax, %eax
	jz uwu0
# true block
	mov $1, %eax
	jmp uwu2
uwu0:
# false blk
	mov $2, %eax
uwu2:

	mov %rbp, %rsp
	pop %rbp
	ret
	
	xor %eax, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
