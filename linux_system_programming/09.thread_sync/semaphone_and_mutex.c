/*******************************************************************************
 *      file name: semaphone_and_mutex.c                                               
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

int buffer[BUFFER_SIZE];        // 共享缓冲区
int count = 0;                  // 缓冲区中的元素数量
int in = 0;                     // 生产者放入位置
int out = 0;                    // 消费者取出位置

sem_t empty;                    // 空闲槽位信号量
sem_t full;                     // 已占用槽位信号量
pthread_mutex_t mutex;          // 互斥锁，保护缓冲区

// 生产者线程函数
void* producer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 5; i++) {
        // 生产一个项目
        int item = rand() % 100;
        
        // 等待空槽位
        sem_wait(&empty);
        
        // 进入临界区
        pthread_mutex_lock(&mutex);
        
        // 放入项目到缓冲区
        buffer[in] = item;
        printf("Producer %d: inserted %d at position %d\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        count++;
        
        // 离开临界区
        pthread_mutex_unlock(&mutex);
        
        // 通知有新项目可用
        sem_post(&full);
        
        // 模拟生产时间
        usleep(rand() % 100000);
    }
    return NULL;
}

// 消费者线程函数
void* consumer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 5; i++) {
        // 等待有项目可用
        sem_wait(&full);
        
        // 进入临界区
        pthread_mutex_lock(&mutex);
        
        // 从缓冲区取出项目
        int item = buffer[out];
        printf("Consumer %d: removed %d from position %d\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        count--;
        
        // 离开临界区
        pthread_mutex_unlock(&mutex);
        
        // 通知有空槽位
        sem_post(&empty);
        
        // 模拟消费时间
        usleep(rand() % 150000);
    }
    return NULL;
}

int main() {
    pthread_t producers[2], consumers[2];
    int producer_ids[2] = {1, 2};
    int consumer_ids[2] = {1, 2};
    
    // 初始化信号量和互斥锁
    sem_init(&empty, 0, BUFFER_SIZE);  // 初始时所有槽位为空
    sem_init(&full, 0, 0);             // 初始时没有占用槽位
    pthread_mutex_init(&mutex, NULL);  // 初始化互斥锁
    
    // 创建生产者和消费者线程
    for (int i = 0; i < 2; i++) {
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < 2; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }
    
    // 销毁信号量和互斥锁
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    
    return 0;
}
