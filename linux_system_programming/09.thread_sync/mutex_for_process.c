/*******************************************************************************
 *      file name: mutex_for_process.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/05/15- 5:05                                    
 * modified time: 25/05/15- 5:05                                    
 *******************************************************************************/
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
struct mt {
	int num;
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
};
int main(void)
{
	int fd, i;
	struct mt *mm;
	pid_t pid;
	fd = open("mt_test", O_CREAT | O_RDWR, 0777);
	/* 不需要write,文件里初始值为0 */
	ftruncate(fd, sizeof(*mm));
	mm = mmap(NULL, sizeof(*mm), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	memset(mm, 0, sizeof(*mm));
	/* 初始化互斥对象属性 */
	pthread_mutexattr_init(&mm->mutexattr);
	/* 设置互斥对象为PTHREAD_PROCESS_SHARED共享，即可以在多个进程的线程访问,PTHREAD_PROCESS_PRIVATE
	   为同一进程的线程共享 */
	pthread_mutexattr_setpshared(&mm->mutexattr,PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&mm->mutex, &mm->mutexattr);
	pid = fork();
	if (pid == 0){
		/* 加10次。相当于加10 */
		for (i=0;i<10;i++){
			pthread_mutex_lock(&mm->mutex);
			(mm->num)++;
			printf("num++:%d\n",mm->num);
			pthread_mutex_unlock(&mm->mutex);
			sleep(1);
		}
	}
	else if (pid > 0) {
		/* 父进程完成x+2,加10次，相当于加20 */
		for (i=0; i<10; i++){
			pthread_mutex_lock(&mm->mutex);
			mm->num += 2;
			printf("num+=2:%d\n",mm->num);
			pthread_mutex_unlock(&mm->mutex);
			sleep(1);
		}
		wait(NULL);
	}
	pthread_mutex_destroy(&mm->mutex);
	pthread_mutexattr_destroy(&mm->mutexattr);
	/* 父子均需要释放 */
	munmap(mm,sizeof(*mm));
	unlink("mt_test");
	return 0;
}
