/*******************************************************************************
 *      File name: pthread_attr_init.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/16-9月:09                                    
 * Modified Time: 22/09/16-9月:09                                    
 *******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void *th_fun(void *arg) {
	int n = 10;
	while(n--) {
		printf("%x    %d\n", (int)pthread_self(), n);
		sleep(1);
	}
	return (void *)1;
}

int main() {
	pthread_t tid;
	pthread_attr_t attr;
	int err;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	err = pthread_create(&tid, &attr, th_fun, NULL);
	printf("phtread_create err:%d\n", err);
	//pthread_attr_destroy(&attr);

	//err =  pthread_join(tid, NULL);
	while(1) {
		if (err != 0) {
			printf("%s\n", strerror(err));
			pthread_exit((void*)1);
		}
		sleep(1);
	}
	return 0;

}

