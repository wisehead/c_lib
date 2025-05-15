/*******************************************************************************
 *      file name: semaphone_test.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/05/15- 5:05                                    
 * modified time: 25/05/15- 5:05                                    
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define PRODUCE_COUNT 20  // 每个生产者生产的项目数

int buffer[BUFFER_SIZE];  // 共享缓冲区
int in = 0;               // 生产者放入位置
int out = 0;              // 消费者取出位置

sem_t empty;              // 空闲槽位信号量（初始值=BUFFER_SIZE）
sem_t full;               // 已占用槽位信号量（初始值=0）

// 生产者线程函数
void* producer(void* arg) {
    for (int i = 0; i < PRODUCE_COUNT; i++) {
        // 生产一个项目
        int item = rand() % 1000 + 1;
        
        // 等待空槽位
        sem_wait(&empty);
        
        // 放入项目到缓冲区（无需mutex保护，因信号量保证互斥）
        buffer[in] = item;
        printf("Producer: produced %d at position %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;
        
        // 通知有新项目可用
        sem_post(&full);
        
        // 模拟生产时间
        usleep(rand() % 100000);
    }
    return NULL;
}

// 消费者线程函数
void* consumer(void* arg) {
    int consumed = 0;
    while (consumed < PRODUCE_COUNT) {  // 消费指定数量后退出
        // 等待有项目可用
        sem_wait(&full);
        
        // 从缓冲区取出项目（无需mutex保护）
        int item = buffer[out];
        printf("Consumer: consumed %d from position %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;
        consumed++;
        
        // 通知有空槽位
        sem_post(&empty);
        
        // 模拟消费时间
        usleep(rand() % 150000);
    }
    return NULL;
}

int main() {
    pthread_t producer_tid, consumer_tid;
    
    // 初始化信号量
    sem_init(&empty, 0, BUFFER_SIZE);  // 初始时所有槽位为空
    sem_init(&full, 0, 0);             // 初始时没有占用槽位
    
    // 创建生产者和消费者线程
    pthread_create(&producer_tid, NULL, producer, NULL);
    pthread_create(&consumer_tid, NULL, consumer, NULL);
    
    // 等待线程完成
    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);
    
    // 销毁信号量
    sem_destroy(&empty);
    sem_destroy(&full);
    
    return 0;
}
