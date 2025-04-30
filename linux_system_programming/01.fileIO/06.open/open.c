/*******************************************************************************
 *      file name: open.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/04/30- 4:04                                    
 * modified time: 25/04/30- 4:04                                    
 *******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

// 以只读模式打开文件并读取内容
void readFile() {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        perror("打开文件失败");
        return;
    }

    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    if (bytesRead == -1) {
        perror("读取文件失败");
    }

    if (close(fd) == -1) {
        perror("关闭文件失败");
    }
}

// 以写入模式打开文件并写入内容
void writeFile() {
    int fd;
    const char *message = "这是写入文件的内容。\n";

    fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("打开文件失败");
        return;
    }

    ssize_t bytesWritten = write(fd, message, strlen(message));
    if (bytesWritten == -1) {
        perror("写入文件失败");
    }

    if (close(fd) == -1) {
        perror("关闭文件失败");
    }
}

// 以追加模式打开文件并追加内容
void appendFile() {
    int fd;
    const char *message = "这是追加到文件的内容。\n";

    fd = open("test.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("打开文件失败");
        return;
    }

    ssize_t bytesWritten = write(fd, message, strlen(message));
    if (bytesWritten == -1) {
        perror("写入文件失败");
    }

    if (close(fd) == -1) {
        perror("关闭文件失败");
    }
}

int main() {
    // 示例调用
    writeFile();
    readFile();
    appendFile();
    return 0;
}    

