/*******************************************************************************
 *      file name: test.c                                               
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
#include <stdatomic.h>

#define BUFFER_SIZE 5
#define PRODUCE_COUNT 20

int buffer[BUFFER_SIZE];
atomic_int in = 0;
atomic_int out = 0;

sem_t empty;
sem_t full;

void* producer(void* arg) {
    for (int i = 0; i < PRODUCE_COUNT; i++) {
        int item = rand() % 1000 + 1;
        
        // 等待空槽位
        sem_wait(&empty);
        
        // 获取当前索引位置
        int index = in;
        
        // 写入数据
        buffer[index] = item;
        
        // 打印生产信息（确保数据已写入）
        printf("Producer: produced %d at position %d\n", item, index);
        
        // 更新索引（原子操作）
        atomic_fetch_add(&in, 1);
        in = in % BUFFER_SIZE;
        
        // 通知有新项目可用
        sem_post(&full);
        
        // 模拟生产时间
        usleep(rand() % 100000);
    }
    return NULL;
}

void* consumer(void* arg) {
    int consumed = 0;
    while (consumed < PRODUCE_COUNT) {
        // 等待有项目可用
        sem_wait(&full);
        
        // 获取当前索引位置
        int index = out;
        
        // 读取数据
        int item = buffer[index];
        
        // 打印消费信息（确保数据已读取）
        printf("Consumer: consumed %d from position %d\n", item, index);
        
        // 更新索引（原子操作）
        atomic_fetch_add(&out, 1);
        out = out % BUFFER_SIZE;
        
        // 通知有空槽位
        sem_post(&empty);
        
        // 增加消费计数
        consumed++;
        
        // 模拟消费时间
        usleep(rand() % 150000);
    }
    return NULL;
}

int main() {
    pthread_t producer_tid, consumer_tid;
    
    // 初始化信号量
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    
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
