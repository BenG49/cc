int main() {
    int a = 0;
    int b = 2;

    {
        int a = 1;
        b = a;
    }

    return b;
}
