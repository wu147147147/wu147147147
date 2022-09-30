#include<stdio.h>
#include<string.h>

int main()
{
    int N,M,i,j;
    int a[1000]={0};
    int c,b,n,m;
    int n1,n2;
    i = 0;
    n = 0;
    j = 0;
    scanf("%d %d",&N,&M);

    while (i<N){
        a[i] = i+1;
        i =i+1;
    }

    i = 0;
    while(i<M)
    {
        scanf("%d %d",&b,&c);
        m = 1;
        while (m<=N){
                if (a[m-1] == b){
                n1 = m-1;

                }

                if (a[m-1] == c){
                    n2 = m-1;
                    }
                m = m + 1;
            }
             j = a[n1];
            a[n1] = a[n2];
            a[n2] = j;
        i = i + 1;
        c,b = 0;
    }
    i = 0;

    while (n<N){
            if (a[n]==0){
            n = N;
            }
            else {
                printf("%d ",a[n]);
            n = n +1;
            }
    }
        n = 0;
    printf("\n");

    return 0;
}
