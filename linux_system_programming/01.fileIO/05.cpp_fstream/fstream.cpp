/*******************************************************************************
 *      file name: fstream.cpp                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/04/30- 4:04                                    
 * modified time: 25/04/30- 4:04                                    
 *******************************************************************************/
#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::fstream file("example.txt", std::ios::in | std::ios::out);
    if (file.is_open()) {
        std::string line;
        // 读取文件内容
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }
        // 移动文件指针到文件开头
        file.seekp(0, std::ios::beg);
        // 写入新内容
        file << "这是新写入的内容。" << std::endl;
        file.close();
    } else {
        std::cerr << "无法打开文件" << std::endl;
    }
    return 0;
}
