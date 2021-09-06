.data

.text

.globl sqrt
sqrt:
	push %rbp
	mov %rsp, %rbp
	sub $12, %rsp
	mov $1, %eax
	push %rax
	mov %rdi, %rax
	pop %rcx
	sar %cl, %eax
	movl %eax, -4(%rbp)
	movl -4(%rbp), %eax
	test %eax, %eax
	mov $0, %eax
	setz %al
	test %eax, %eax
	jz sus0
	mov %rdi, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
sus0:
	mov $1, %eax
	push %rax
	movl -4(%rbp), %eax
	push %rax
	mov %rdi, %rax
	pop %rcx
	cdq
	idiv %ecx
	push %rax
	movl -4(%rbp), %eax
	pop %rcx
	add %ecx, %eax
	pop %rcx
	sar %cl, %eax
	movl %eax, -8(%rbp)
sus1:
	movl -4(%rbp), %eax
	push %rax
	movl -8(%rbp), %eax
	pop %rcx
	cmp %ecx, %eax
	mov $0, %eax
	setl %al
	test %eax, %eax
	jz sus2
	movl -8(%rbp), %eax
	movl %eax, -4(%rbp)
	mov $1, %eax
	push %rax
	movl -4(%rbp), %eax
	push %rax
	mov %rdi, %rax
	pop %rcx
	cdq
	idiv %ecx
	push %rax
	movl -4(%rbp), %eax
	pop %rcx
	add %ecx, %eax
	pop %rcx
	sar %cl, %eax
	movl %eax, -8(%rbp)
	jmp sus1
sus2:
	movl -4(%rbp), %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret

.globl low_factor
low_factor:
	push %rbp
	mov %rsp, %rbp
	sub $8, %rsp
	mov $1, %eax
	push %rax
	mov %rdi, %rax
	pop %rcx
	and %ecx, %eax
	test %eax, %eax
	mov $0, %eax
	setz %al
	test %eax, %eax
	jz sus3
	mov $2, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
sus3:
	mov %rdi, %rax
	push %rdi
	mov %rax, %rdi
	call sqrt
	pop %rdi
	movl %eax, -4(%rbp)
	sub $4, %rsp
	mov $3, %eax
	movl %eax, -8(%rbp)
sus4:
	movl -4(%rbp), %eax
	push %rax
	movl -8(%rbp), %eax
	pop %rcx
	cmp %ecx, %eax
	mov $0, %eax
	setl %al
	test %eax, %eax
	jz sus6
	mov $0, %eax
	push %rax
	movl -8(%rbp), %eax
	push %rax
	mov %rdi, %rax
	pop %rcx
	cdq
	idiv %ecx
	mov %edx, %eax
	pop %rcx
	cmp %ecx, %eax
	mov $0, %eax
	sete %al
	test %eax, %eax
	jz sus7
	movl -8(%rbp), %eax
	mov %rbp, %rsp
	pop %rbp
	ret
sus7:
sus5:
	movl -8(%rbp), %eax
	inc %eax
	movl %eax, -8(%rbp)
	jmp sus4
sus6:
	add $4, %rsp
	mov $1, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret

.globl gpf
gpf:
	push %rbp
	mov %rsp, %rbp
	sub $16, %rsp
	mov %rdi, %rax
	push %rdi
	mov %rax, %rdi
	call low_factor
	pop %rdi
	movl %eax, -4(%rbp)
	mov $1, %eax
	push %rax
	movl -4(%rbp), %eax
	pop %rcx
	cmp %ecx, %eax
	mov $0, %eax
	sete %al
	test %eax, %eax
	jz sus8
	mov %rdi, %rax
	mov %rbp, %rsp
	pop %rbp
	ret
sus8:
	movl -4(%rbp), %eax
	push %rdi
	mov %rax, %rdi
	call gpf
	pop %rdi
	movl %eax, -8(%rbp)
	movl -4(%rbp), %eax
	push %rax
	mov %rdi, %rax
	pop %rcx
	cdq
	idiv %ecx
	push %rdi
	mov %rax, %rdi
	call gpf
	pop %rdi
	movl %eax, -12(%rbp)
	movl -12(%rbp), %eax
	push %rax
	movl -8(%rbp), %eax
	pop %rcx
	cmp %ecx, %eax
	mov $0, %eax
	setg %al
	test %eax, %eax
	jz sus9
	movl -8(%rbp), %eax
	jmp sus11
sus9:
	movl -12(%rbp), %eax
sus11:
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
	mov $13195, %eax
	push %rdi
	mov %rax, %rdi
	call gpf
	pop %rdi
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
