/*******************************************************************************
 *      File name: rwlock.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/17-9月:09                                    
 * Modified Time: 22/09/17-9月:09                                    
 *******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int counter;
pthread_rwlock_t rwlock;
//3个线程不定时写同一全局资源，5个线程不定时读同一全局资源 
void *th_write(void *arg)
{
	int t;
	while (1) { 
		pthread_rwlock_wrlock(&rwlock);
		t = counter;
		usleep(100);
		printf("write %x : counter=%d ++counter=%d\n", (int)pthread_self(), t, ++counter);
		pthread_rwlock_unlock(&rwlock);
		usleep(100);
	} 
}

void *th_read(void *arg) {
	while (1) {
		pthread_rwlock_rdlock(&rwlock);
		printf("read %x : %d\n", (int)pthread_self(), counter);
		pthread_rwlock_unlock(&rwlock);
		usleep(100);
	} 
}

int main(void) {
	int i;
	pthread_t tid[8];
	pthread_rwlock_init(&rwlock, NULL);
	for (i = 0; i < 3; i++)
		pthread_create(&tid[i], NULL, th_write, NULL);
	for (i = 0; i < 5; i++)
		pthread_create(&tid[i+3], NULL, th_read, NULL);
	pthread_rwlock_destroy(&rwlock);
	for (i = 0; i < 8; i++)
		pthread_join(tid[i], NULL);
	return 0;
}

