int main() {
    int sum = 0;
    for (int i = 0; i <= 10; i++)
    {
        if (i == 5)
            break;
        sum += i;
    }
    return sum;
}
