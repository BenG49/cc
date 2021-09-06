int sqrt(int n)
{
    // init est
    int x0 = n >> 1;

    if (!x0)
        return n;
    
    // update
    int x1 = ( x0 + n / x0 ) >> 1;
		
    while ( x1 < x0 )
    {
        x0 = x1;
        x1 = ( x0 + n / x0 ) >> 1;
    }
		
    return x0;
}

int low_factor(int n)
{
    if (!(n & 1))
        return 2;
    
    int s = sqrt(n);

    for (int i = 3; i < s; ++i)
        if (n % i == 0)
            return i;
    
    return 1;
}

int gpf(int n)
{
    int f = low_factor(n);

    if (f == 1)
        return n;
    
    int a = gpf(f);
    int b = gpf(n / f);

    return (a > b) ? a : b;
}

int main()
{
    return gpf(13195);
}
