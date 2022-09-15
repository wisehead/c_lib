/*******************************************************************************
 *      File name: setpgid.c                                               
 *         Author: Hui Chen. (c) 2022                             
 *           Mail: alex.chenhui@gmail.com                                        
 *   Created Time: 22/09/15-9月:09                                    
 * Modified Time: 22/09/15-9月:09                                    
 *******************************************************************************/

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
int main(void) {
	pid_t pid;

	if ((pid = fork()) < 0) { perror("fork");
		exit(1);
	} else if (pid == 0) {
		printf("child process PID is %d\n",getpid());
		printf("Group ID of child is %d\n",getpgid(0));
		// 返回组id sleep(5);
		sleep(5);

		printf("Group ID of child is changed to %d\n",getpgid(0));
		exit(0);
	}
	sleep(1);

	setpgid(pid,pid);
	// 父进程改变子进程的组id为子进程本身
	sleep(5);

	printf("parent process PID is %d\n",getpid());

	printf("parent of parent process PID is %d\n",getppid());
	printf("Group ID of parent is %d\n",getpgid(0));
	setpgid(getpid(),getppid());
	// 改变父进程的组id为父进程的父进程 printf("Group ID of parent is changed to %d\n",getpgid(0));

	return 0;
}
