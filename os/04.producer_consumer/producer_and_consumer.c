/*******************************************************************************
 *      file name: producer_and_consumer.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/11/14-15:37:09                              
 *  modified time: 2020/11/14-15:37:09                              
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
//#include <error.h>
#include <sys/errno.h>
#include <pthread.h>
//产品
typedef struct Msg{
    struct Msg* next;
    int num;
}Msg;

pthread_mutex_t lock =PTHREAD_MUTEX_INITIALIZER; //静态初始化mutex 互斥锁，也可以使用pthread_mutex_init方法进出初始化
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;  //初始化条件变量
Msg* head = NULL;

//生产者
void* produce(void* arg)
{
    Msg* msg = NULL;
    int i = (int)arg;//序号
    while(1)
    {
        msg = (Msg*)malloc(sizeof(Msg));
        msg->num = rand()%1000;
        //添加到公共区域
        pthread_mutex_lock(&lock);
        msg->next = head;
        head = msg;
        pthread_mutex_unlock(&lock);
 
        printf("%dth ***producer***:%d\n",i,msg->num);
        pthread_cond_broadcast(&cond);
        sleep(rand()%3);
    }
    return NULL;
}

void* custome(void* arg)
{
    Msg* msg = NULL;
    int i = (int)arg;//序号
    while(1)
    {
        pthread_mutex_lock(&lock);
        //为什么 while而不是if ?当没有产品时,多个消费者线程有可能同时阻塞在cond_wait。
        //当产品区有一个产品，被唤醒的一个消费者将产品消费，其他产品再次消费时，需要判断产品区是否有产品
        while( head == NULL )
        {
            pthread_cond_wait(&cond,&lock);
        }
        //消费
        msg = head;
        head = msg->next;
        pthread_mutex_unlock(&lock);
 
        printf("%dth ###customer###:%d\n",i,msg->num);
        free(msg);
        sleep(rand()%3);
    }
    return NULL;
}

int main(int argc,char* argv[])
{
    srand((unsigned int)time(NULL));
    pthread_t pro[3],cus[5];
    int i;
    //3 个生产者线程
    for(i = 0 ; i <3; i++)
    {
        pthread_create(&pro[i],NULL,produce,(void*)i);
    }
    //5 个消费者线程
    for( i = 0; i < 5; i++ )
    {
        pthread_create(&cus[i],NULL,custome,(void*)i);
    }
    pthread_mutex_destroy(&lock);
    //主线程回收线程
    for( i = 0; i < 3; i++ )
    {
        pthread_join(pro[i],NULL);
    }
    for( i = 0; i < 5; i++ )
    {
        pthread_join(cus[i],NULL);
    }
    return 0;
}
