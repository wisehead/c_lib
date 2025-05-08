/*******************************************************************************
 *      file name: setMandatoryLock.cpp                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/05/08- 5:05                                    
 * modified time: 25/05/08- 5:05                                    
 *******************************************************************************/
#include <iostream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string>

// 加强制锁
bool setMandatoryLock(int fd, int type) {
    struct flock lock;
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        return false;
    }
    return true;
}

int main() {
    std::string filename = "test.txt";
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 02666);  // 设置 set - group - ID 位
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 加写锁
    if (setMandatoryLock(fd, F_WRLCK)) {
        std::cout << "已加强制写锁，开始写入数据..." << std::endl;
        // 模拟写入操作
        write(fd, "Hello, Mandatory File Lock!", 22);
        std::cout << "数据写入完成，释放强制写锁..." << std::endl;
        // 解锁
        setMandatoryLock(fd, F_UNLCK);
    }

    close(fd);
    return 0;
}
