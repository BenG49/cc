	
.globl main
main:
	push %rbp
	mov %rsp, %rbp
	sub $4, %rsp
	
	mov $2, %eax
	mov %eax, -4(%rbp)
	sub $4, %rsp
	
	mov $3, %eax
	mov %eax, -4(%rbp)
	mov $0, %eax
	mov %eax, -8(%rbp)
	
	add $4, %rsp
	movl -4(%rbp), %eax

	mov %rbp, %rsp
	pop %rbp
	ret
	
	xor %eax, %eax

	mov %rbp, %rsp
	pop %rbp
	ret
	
