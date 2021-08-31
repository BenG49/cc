.globl main
main:
	mov $18, %rax
	not %rax
	neg %rax
	test %rax, %rax
	mov $0, %rax
	setz %al
	ret
