/*******************************************************************************
 *      file name: setAdvisoryLock.cpp                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/05/08- 5:05                                    
 * modified time: 25/05/08- 5:05                                    
 *******************************************************************************/
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string>

// 加建议锁
bool setAdvisoryLock(int fd, int type) {
    struct flock lock;
    lock.l_type = type;  // 锁的类型：F_RDLCK（读锁）、F_WRLCK（写锁）、F_UNLCK（解锁）
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // 锁定整个文件

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        return false;
    }
    return true;
}

int main() {
    std::string filename = "test.txt";
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 加写锁
    if (setAdvisoryLock(fd, F_WRLCK)) {
        std::cout << "已加写锁，开始写入数据..." << std::endl;
        // 模拟写入操作
        write(fd, "Hello, File Lock!", 16);
        std::cout << "数据写入完成，释放写锁..." << std::endl;
        // 解锁
        setAdvisoryLock(fd, F_UNLCK);
    }

    close(fd);
    return 0;
}
