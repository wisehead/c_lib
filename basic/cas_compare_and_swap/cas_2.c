/*******************************************************************************
 *      file name: cas_2.c                                               
 *         author: Hui Chen. (c) 2018                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2018/12/28-16:59:35                              
 *  modified time: 2018/12/28-16:59:35                              
 *******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
 
int sum = 0;
 
void* adder(void *p)
{
    int old = sum;
    for(int i = 0; i < 1000000; i++)  // 百万次
    {
        while(!__sync_bool_compare_and_swap(&sum, old, old + 1))  // 如果old等于sum, 就把old+1写入sum
        {
           old = sum; // 更新old
        }
    }
 
    return NULL;
}
 
int main()
{
    pthread_t threads[10];
    for(int i = 0;i < 10; i++)
    {
        pthread_create(&threads[i], NULL, adder, NULL);
    }
	
    for(int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }
 
    printf("sum is %d\n",sum);
}

