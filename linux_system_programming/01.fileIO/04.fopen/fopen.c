/*******************************************************************************
 *      file name: fopen.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/04/30- 4:04                                    
 * modified time: 25/04/30- 4:04                                    
 *******************************************************************************/
#include <stdio.h>
#include <iostream>
#include <cstdio>

int main() {
    FILE* file = fopen("example.txt", "r");
    if (file != nullptr) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file) != nullptr) {
            std::cout << buffer;
        }
        fclose(file);
    } else {
        std::cerr << "无法打开文件" << std::endl;
    }
    return 0;
}
