	
.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $4, %rsp
	
	mov $0, %eax
	mov %eax, -4(%rbp)
	sub $4, %rsp
	mov $0, %eax
	mov %eax, -8(%rbp)
sus0:
	mov $10, %eax
	push %rax
	movl -8(%rbp), %eax
	pop %rcx
	cmp %ecx, %eax
	mov $0, %eax
	setl %al
	test %eax, %eax
	jz sus2
	
	mov $2, %eax
	push %rax
	movl -4(%rbp), %eax
	pop %rcx
	cdq
	idiv %ecx
	mov %edx, %eax
	test %eax, %eax
	jz sus3
	jmp sus1
sus3:
	movl -8(%rbp), %eax
	push %rax
	movl -4(%rbp), %eax
	pop %rcx
	add %ecx, %eax
	mov %eax, -4(%rbp)
	
sus1:
	movl -8(%rbp), %eax
	inc %eax
	mov %eax, -8(%rbp)
	jmp sus0
sus2:
	add $4, %rsp
	movl -4(%rbp), %eax

	mov %rbp, %rsp
	pop %rbp
	ret
	
	xor %eax, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
	
