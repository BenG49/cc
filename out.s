.data

.text

.globl main
main:
	push %rbp
	mov %rsp, %rbp
	mov $72, %al
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $105, %al
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $10, %al
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
