.data

.text

.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $1, %rsp
	movl %eax, -1(%rbp)
	movl -1(%rbp), %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
