/*******************************************************************************
 *      file name: cstar_ucstar.c                                               
 *         author: Hui Chen. (c) 2019                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2019/12/17-13:48:55                              
 *  modified time: 2019/12/17-13:48:55                              
 *******************************************************************************/
#include <stdio.h>
int main(int argc, char *argv[])
{
    unsigned char k = 0;
    int i = -1;
    short a = -12345;
    char *p;
    unsigned char *q;
 
    printf("sizeof(i) = %d\n",sizeof(i));
    printf("sizeof(a) = %d\n",sizeof(a));
    printf("-----------------------------\n");
    printf("begin p(char):\n");
    p = (char*)&a;
    printf("a = %u | %d\n",a,a);
    for(k=0;k<sizeof(a);k++)
    {
        printf("0x%x ",*(p++));
    }
    printf("\n");
    p = (char*)&i;
    printf("i = %u | %d\n",i,i);
    for(k=0;k<sizeof(i);k++)
    {
        printf("0x%x ",*(p++));
    }
    printf("\n");
    printf("-1 > 0u: %s\n",(-1>0u ? "true":"false"));
 
    printf("-----------------------------\n");
    printf("begin q(unsigned char):\n");
    q = (unsigned char*)&a;
    printf("a = %u | %d\n",a,a);
    for(k=0;k<sizeof(a);k++)
    {
        printf("0x%x ",*(q++));
    }
    printf("\n");
    q = (unsigned char*)&i;
    printf("i = %u | %d\n",i,i);
    for(k=0;k<sizeof(i);k++)
    {
        printf("0x%x ",*(q++));
    }
    printf("\n");
    printf("-1 > 0u: %s\n",(-1>0u ? "true":"false"));
 
    return 0;
}
