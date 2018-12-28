/*******************************************************************************
 *      file name: cas_0_nolatch.c                                               
 *         author: Hui Chen. (c) 2018                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2018/12/28-16:57:41                              
 *  modified time: 2018/12/28-16:57:41                              
 *******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
 
int sum = 0;
 
void* adder(void *p)
{
    for(int i = 0; i < 1000000; i++)  // 百万次
    {
        sum++;
    }
 
    return NULL;
}
 
int main()
{
    pthread_t threads[10];
 
    for(int i = 0; i < 10; i++)
    {
        pthread_create(&threads[i], NULL, adder, NULL);
    }
	
    for(int i = 0; i < 10; i++)
    {
        pthread_join(threads[i],NULL);
    }
 
	printf("sum is %d\n", sum);
}

