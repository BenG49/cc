
int main()
{
    int sum = 0;
    for (int i = 0; i < 10; ++i)
    {
        if (sum % 2)
            continue;

        sum += i;
    }

    return sum;
}
