/*******************************************************************************
 *      file name: perror.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/04/30- 4:04                                    
 * modified time: 25/04/30- 4:04                                    
 *******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd;
    // 尝试打开一个不存在的文件
    fd = open("nonexistent_file.txt", O_RDONLY);
    if (fd == -1) {
        perror("打开文件失败");
        return 1;
    }
    // 若文件成功打开，关闭文件
    close(fd);
    return 0;
}
