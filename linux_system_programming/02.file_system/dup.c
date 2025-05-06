/*******************************************************************************
 *      file name: dup.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/05/06- 5:05                                    
 * modified time: 25/05/06- 5:05                                    
 *******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void)
{
	int fd, save_fd;
	char msg[] = "This is a test\n";
	fd = open("somefile", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	if(fd<0) {
		perror("open");
		exit(1);
	}
	save_fd = dup(STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	write(STDOUT_FILENO, msg, strlen(msg));
	dup2(save_fd, STDOUT_FILENO);
	write(STDOUT_FILENO, msg, strlen(msg));
	close(save_fd);
	return 0;
}
