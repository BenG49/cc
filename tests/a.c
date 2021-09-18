char f1(int input) { return input; }
int f2(char input) { return input; }

int main()
{
    char i1 = 'a';
    int i2 = 69;
    return f1(i1) + f2(i2);
}
