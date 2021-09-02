.globl main
main:
	push %rbp
	mov %rsp, %rbp
	
	mov $2, %eax
	push %rax
	mov $2, %eax
	push %rax
	mov $1, %eax
	pop %rcx
	xor %ecx, %eax
	pop %rcx
	and %ecx, %eax
	push %rax
	mov $2, %eax
	pop %rcx
	cmp %eax, %ecx
	mov $0, %eax
	sete %al

	mov %rbp, %rsp
	pop %rbp
	ret
	
	xor %eax, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
