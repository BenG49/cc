.globl f1
f1:
	movl %edi, %eax
	ret
.globl f2
f2:
	movb %dil, %al
	ret
.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $5, %rsp

	# i1 = 'a'
	movb $97, %r10b
	movb %r10b, -1(%rbp)
	# i2 = 69
	movl $69, %r10d
	movl %r10d, -5(%rbp)
	# call f1 with i1
	movb -1(%rbp), %r10b
	push %rdi
	movq %r10, %rdi
	call f1
	pop %rdi
	movq %rax, %r10

	push %r10
	movl -5(%rbp), %r11d
	push %rdi
	movq %r11, %rdi
	call f2
	pop %rdi
	pop %r10
	movq %rax, %r11

	add %r10, %r11
	movb %r11b, %al
	mov %rbp, %rsp
	pop %rbp
	ret
