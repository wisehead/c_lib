/*******************************************************************************
 *      file name: getuid.c                                               
 *         author: hui chen. (c) 22                             
 *           mail: chenhui13@xxx.com                                        
 *   created time: 22/09/15-9月:09                                    
 * modified time: 22/09/15-9月:09                                    
 *******************************************************************************/
#include <stdio.h>
#include <sys/types.h>                                      
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

int main() {
	int fd;
	//fd = open("abc", O_CREAT, 0777);
	fd = open("abc", O_CREAT, 04777);
	printf("uid: %d\n", getuid());
	printf("euid: %d\n", geteuid());
	
	close(fd);
	return 0;
}
