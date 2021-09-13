.globl sum
sum:
	push %rbp
	mov %rsp, %rbp
	movl %edi, %r10d
	movl %esi, %eax
	add %r10, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
.globl main
main:
	push %rbp
	mov %rsp, %rbp
	movq $1, %rdi
	movq $2, %rsi
	call sum
	movq %rax, %r10
	movq $1, %rdi
	movq $2, %rsi
	call sum
	movq %rax, %r11

	movq $2, %r12
	movq %r11, %rax
	cqo
	idiv %r12
	movq %rax, %r11

	movq $2, %r12
	imul %r11, %r12
	sub %r12, %r10
	movl %r10d, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
