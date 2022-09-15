/*******************************************************************************
 *      file name: getpgid.c                                               
 *         author: hui chen. (c) 22                             
 *           mail: chenhui13@xxx.com                                        
 *   created time: 22/09/15-9月:09                                    
 * modified time: 22/09/15-9月:09                                    
 *******************************************************************************/
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
int main(void) {
	pid_t pid;

	if ((pid = fork()) < 0) { perror("fork");
		exit(1);
	}else if (pid == 0) {
		printf("child process PID is %d\n",getpid());
		printf("Group ID is %d\n",getpgrp());
		printf("Group ID is %d\n",getpgid(0));
		printf("Group ID is %d\n",getpgid(getpid()));
		exit(0);
	}
	sleep(3);

	printf("parent process PID is %d\n",getpid());
	printf("Group ID is %d\n",getpgrp());
	wait(NULL);

	return 0;
}
