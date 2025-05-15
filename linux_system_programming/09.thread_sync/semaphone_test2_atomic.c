/*******************************************************************************
 *      file name: semaphone_test2_atomic.c                                               
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
        
        sem_wait(&empty);
        
        int index = atomic_fetch_add(&in, 1) % BUFFER_SIZE;
        buffer[index] = item;
        
        printf("Producer: produced %d at position %d\n", item, index);
        
        sem_post(&full);
        
        usleep(rand() % 100000);
    }
    return NULL;
}

void* consumer(void* arg) {
    int consumed = 0;
    while (consumed < PRODUCE_COUNT) {
        sem_wait(&full);
        
        int index = atomic_fetch_add(&out, 1) % BUFFER_SIZE;
        int item = buffer[index];
        
        printf("Consumer: consumed %d from position %d\n", item, index);
        consumed++;
        
        sem_post(&empty);
        
        usleep(rand() % 150000);
    }
    return NULL;
}

int main() {
    pthread_t producer_tid, consumer_tid;
    
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    
    pthread_create(&producer_tid, NULL, producer, NULL);
    pthread_create(&consumer_tid, NULL, consumer, NULL);
    
    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);
    
    sem_destroy(&empty);
    sem_destroy(&full);
    
    return 0;
}
