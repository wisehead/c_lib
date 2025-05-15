/*******************************************************************************
 *      File name: semaphone.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/17-9月:09                                    
 * Modified Time: 22/09/17-9月:09                                    
 *******************************************************************************/
#include <stdlib.h> 
#include <pthread.h> 
#include <stdio.h> 
#include <semaphore.h>
#include <unistd.h>

#define NUM 5

int queue[NUM];
sem_t blank_number, product_number;
pthread_mutex_t mutex; 

void *producer(void *arg) {
	int p = 0;
	while (1) {
		sem_wait(&blank_number);
		pthread_mutex_lock(&mutex);
		queue[p] = rand() % 1000 + 1;
		printf("Produce %d, p:%d\n", queue[p], p);
		p = (p+1)%NUM;
		pthread_mutex_unlock(&mutex);
		sem_post(&product_number);
		//p = (p+1)%NUM;
		sleep(rand()%5);
	} 
}

void *consumer(void *arg) {
	int c = 0;
	while (1) {
		sem_wait(&product_number);
		pthread_mutex_lock(&mutex);
		printf("Consume %d, c:%d\n", queue[c], c);
		queue[c] = 0;
		c = (c+1)%NUM;
		pthread_mutex_unlock(&mutex);
		sem_post(&blank_number);
		//c = (c+1)%NUM;
		sleep(1);
		//sleep(rand()%5);
	}
}

int main(int argc, char *argv[]) {
	pthread_t pid, cid;
	sem_init(&blank_number, 0, NUM);
	sem_init(&product_number, 0, 0);
	pthread_mutex_init(&mutex, NULL);  // 初始化互斥锁
	pthread_create(&pid, NULL, producer, NULL);
	pthread_create(&cid, NULL, consumer, NULL);
	pthread_join(pid, NULL);
	pthread_join(cid, NULL);
	sem_destroy(&blank_number);
	sem_destroy(&product_number);
	return 0;
}
