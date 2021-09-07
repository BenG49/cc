int a = 10;

int main() {
	int i = 0;

	int *a_ = &a;
	int *i_ = &i;

	*a_ = 5;
	*i_ = 2;
}

/*

.data
.globl a
a: .int 10

.text
.global main
main:
	push %rbp
	mov %rsp, %rbp
	sub $20, %rsp

	# i = 0
	mov $0, %eax
	movl %eax, -4(%rbp)

	# a_ = &a
	leaq a(%rip), $rax
	movq %raq, -12($rbp)

	# i_ = &i
	leaq -4(%rbp), %rax
	movq %rax, -20(%rbp)

	# *a_ = 5
	mov %eax, $5
	mov -12(%rbp), %rbx
	movl %eax, (%rbx)

	# *i_ = 2
	mov %eax, $2
	mov -20(%rbp), %rbx
	movl %eax, (%rbx)

	mov %rbp, %rsp
	pop %rbp
	ret
*/
