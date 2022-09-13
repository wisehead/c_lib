/*******************************************************************************
 *      file name: lseek.c                                               
 *         author: hui chen. (c) 22                             
 *           mail: chenhui13@xxx.com                                        
 *   created time: 22/09/13-9月:09                                    
 * modified time: 22/09/13-9月:09                                    
 *******************************************************************************/
#include <stdio.h>
#include <fcntl.h>

int main(void) {
	int fd = open("abc", O_RDWR);
	if (fd < 0) {
		perror("open abc");
		exit(-1);
	}

	lseek(fd, 0x1000, SEEK_SET);
	write(fd, "a", 1);
	close(fd);
	return 0;
}
