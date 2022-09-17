/*******************************************************************************
 *      File name: addcount.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/17-9月:09                                    
 * Modified Time: 22/09/17-9月:09                                    
 *******************************************************************************/
#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>

#define NLOOP 5000 
int counter;
//int volatile counter;
void *doit(void *);
/* incremented by threads */
int main(int argc, char **argv) {
	pthread_t tidA, tidB;
	pthread_create(&tidA, NULL, &doit, NULL);
	pthread_create(&tidB, NULL, &doit, NULL);
	/* wait for both threads to terminate */ 
	pthread_join(tidA, NULL);
	pthread_join(tidB, NULL);
	return 0;
}
void *doit(void *vptr) {
	int i, val;
	for (i = 0; i < NLOOP; i++) { 
		/*
		val = counter;
		printf("%x: %d\n", (unsigned int)pthread_self(), val + 1);
		counter = val + 1;
		*/
		printf("%x: %d\n", (unsigned int)pthread_self(), ++counter);

	}
	return NULL;
}
