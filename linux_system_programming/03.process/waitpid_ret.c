/*******************************************************************************
 *      file name: waitpid_ret.c                                               
 *         author: hui chen. (c) 22                             
 *           mail: chenhui13@xxx.com                                        
 *   created time: 22/09/15-9月:09                                    
 * modified time: 22/09/15-9月:09                                    
 *******************************************************************************/
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>

int main(void) {
	pid_t pid;
	pid = fork(); if (pid < 0) {
		perror("fork failed");
		exit(1); 
	}
	if (pid == 0) { 
		int i;
		for (i = 3; i > 0; i--) { 
			printf("This is the child\n"); 
			sleep(1);
		}	
		exit(3); 
	} else {
		int stat_val;
		waitpid(pid, &stat_val, 0);
		if (WIFEXITED(stat_val))
			printf("Child exited with code %d\n", WEXITSTATUS(stat_val));
		else if (WIFSIGNALED(stat_val))
			printf("Child terminated abnormally, signal %d\n", WTERMSIG(stat_val));
	}
	return 0; 
}
