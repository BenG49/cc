.globl main
main:
	push %rbp
	mov %rsp, %rbp
	movq $72, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $101, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $108, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $108, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $111, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $44, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $32, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $87, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $111, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $114, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $108, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $100, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $33, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	movq $10, %r10
	push %rdi
	movq %r10, %rdi
	call putchar
	pop %rdi
	movq %rax, %r10
	xor %rax, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
	add $1918986339, %rsp
