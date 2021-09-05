
// test sysv stack abi
int lots_of_args(int a, int b, int c, int d, int e, int f, int stack1, int stack2)
{
    return stack1 + stack2;
}

int putchar(int c);

int main()
{
    // yes
    putchar(72);
    putchar(101);
    putchar(108);
    putchar(108);
    putchar(111);
    putchar(32);
    putchar(87);
    putchar(111);
    putchar(114);
    putchar(108);
    putchar(100);
    putchar(33);
    putchar(10);

    return lots_of_args(1, 2, 3, 4, 5, 6, 7, 8);
}
