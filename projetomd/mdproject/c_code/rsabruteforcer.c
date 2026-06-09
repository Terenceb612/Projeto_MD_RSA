#include <stdio.h>
#include <math.h>

int verificar_primo(long long a)
{
    if (a < 2) return 0; // menores que 2 não são primos

    if (a == 2 || a == 3) return 1; // 2 e 3 são primos

    if (a % 2 == 0 || a % 3 == 0) return 0; // elimina pares e múltiplos de 3

    double limite = sqrt((double)a);

    for (long long i = 5; i <= limite; i += 6) {
        if (a % i == 0 || a % (i + 2) == 0) return 0;
    }
    return 1;
}

void proximo_primo(long long *a)
{
    (*a)+=2;
    while(!verificar_primo(*a))
    {
        (*a)+=2;
    }
}

void geradorp_q(long long *p, long long *q, long long n)
{
    while((*p)*(*q)!=n)
    {
        while((*p)*(*q)<n)
        {
            proximo_primo(q);

        }
        if((*p)*(*q)==n) return;
        (*q) = 3;
        if((*p)==2) (*p) = 3;
        else proximo_primo(p);
    }
}

int main()
{
    long long n, p = 2, q = 3;
    scanf("%lld",&n);
    geradorp_q(&p,&q,n);
    printf("p = %lld, q = %lld\n",p,q);
    return 0;
}