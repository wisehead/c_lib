/*******************************************************************************
 *      File name: pthread_detach.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/16-9月:09                                    
 * Modified Time: 22/09/16-9月:09                                    
 *******************************************************************************/

#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <unistd.h> 
#include <string.h>
void *thr_fn(void *arg)
{
	int n = 3;
	while (n--) {
		printf("thread count %d\n", n);
		sleep(1);
	}
	return (void *)1;
}
int main(void) {
	pthread_t tid;
	void *tret;
	int err;
	pthread_create(&tid, NULL, thr_fn, NULL);
	//第一次运行时注释掉下面这行，第二次再打开，分析两次结果 
	pthread_detach(tid);
	while (1) {
		err = pthread_join(tid, &tret);
		if (err != 0)
			fprintf(stderr, "thread %s\n", strerror(err));
		else
			fprintf(stderr, "thread exit code %d\n", (int)tret);
		sleep(1);
	}
	return 0;
}
