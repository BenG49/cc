	sub $12, %rsp

.globl lots_of_args
lots_of_args:
	push %rbp
	mov %rsp, %rbp
	sub $32, %rsp
	movl 24(%rbp), %eax
	push %rax
	movl 16(%rbp), %eax
	pop %rcx
	add %ecx, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret

.globl main
main:
	push %rbp
	mov %rsp, %rbp
	mov $72, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $101, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $108, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $108, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $111, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $32, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $87, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $111, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $114, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $108, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $100, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $33, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $10, %eax
	push %rdi
	mov %rax, %rdi
	call putchar
	pop %rdi
	mov $1, %eax
	push %rdi
	mov %rax, %rdi
	mov $2, %eax
	push %rsi
	mov %rax, %rsi
	mov $3, %eax
	push %rdx
	mov %rax, %rdx
	mov $4, %eax
	push %rcx
	mov %rax, %rcx
	mov $5, %eax
	push %r8
	mov %rax, %r8
	mov $6, %eax
	push %r9
	mov %rax, %r9
	mov $8, %eax
	push %rax
	mov $7, %eax
	push %rax
	call lots_of_args
	pop %r9
	pop %r8
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	add $8, %rsp
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	add $12, %rsp
