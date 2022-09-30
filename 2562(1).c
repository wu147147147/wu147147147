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

    while (i<N) //建立矩陣
        {
        a[i] = i+1;
        i =i+1;
    }

    i = 0;
    while(i<M) //執行M次的換座位
    {
        scanf("%d %d",&b,&c);
        m = 1;
        while (m<=N) {
                if (a[m-1] == b) //找出數值=b的位置，並紀錄為n1
                {
                n1 = m-1;

                }

                if (a[m-1] == c) //找出數值=c的位置，並紀錄為n2
                    {
                    n2 = m-1;
                    }
                m = m + 1;
            }
             j = a[n1];
            a[n1] = a[n2];
            a[n2] = j; //交換位置
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
