.data

.text

.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $1, %rsp
	mov $48, %al
	movb %al, -1(%rbp)
	mov $1, %eax
	push %rax
	movb -1(%rbp), %al
	pop %rcx
	add %ecx, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
