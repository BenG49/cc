.data
.globl foo
foo: .int 1
.globl a
a: .int 20

.text

.globl main
main:
	push %rbp
	mov %rsp, %rbp
	mov $10, %eax
	movq %rax, foo(%rip)
	movq foo(%rip), %rax
	mov %rbp, %rsp
	pop %rbp
	ret
	xor %eax, %eax
	mov %rbp, %rsp
	pop %rbp
	ret
