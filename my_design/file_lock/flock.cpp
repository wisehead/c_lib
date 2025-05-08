/*******************************************************************************
 *      file name: flock.cpp                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/05/08- 5:05                                    
 * modified time: 25/05/08- 5:05                                    
 *******************************************************************************/
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <string>

int main() {
    std::string filename = "test.txt";
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 加写锁
    if (flock(fd, LOCK_EX) == -1) {
        perror("flock");
        close(fd);
        return 1;
    }
    std::cout << "已加写锁，开始写入数据..." << std::endl;
    // 模拟写入操作
    write(fd, "Hello, Flock!", 12);
    std::cout << "数据写入完成，释放写锁..." << std::endl;
    // 解锁
    if (flock(fd, LOCK_UN) == -1) {
        perror("flock");
    }

    close(fd);
    return 0;
}
