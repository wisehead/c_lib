/*******************************************************************************
 *      file name: ofstream.cpp                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/04/30- 4:04                                    
 * modified time: 25/04/30- 4:04                                    
 *******************************************************************************/
#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ofstream outputFile("example.txt");
    if (outputFile.is_open()) {
        outputFile << "这是写入文件的内容。" << std::endl;
        outputFile.close();
    } else {
        std::cerr << "无法打开文件" << std::endl;
    }
    return 0;
}
