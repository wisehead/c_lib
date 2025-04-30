/*******************************************************************************
 *      file name: rw.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/04/30- 4:04                                    
 * modified time: 25/04/30- 4:04                                    
 *******************************************************************************/
#include <stdio.h>

// 以只读模式打开文件并读取内容
void readFile() {
    FILE *file;
    char buffer[256];

    file = fopen("example.txt", "r");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
}

// 以写入模式打开文件并写入内容
void writeFile() {
    FILE *file;

    file = fopen("example.txt", "w");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    fputs("这是写入文件的内容。\n", file);

    fclose(file);
}

// 以追加模式打开文件并追加内容
void appendFile() {
    FILE *file;

    file = fopen("example.txt", "a");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    fputs("这是追加到文件的内容。\n", file);

    fclose(file);
}

// 以二进制只读模式打开文件并读取内容
void readBinaryFile() {
    FILE *file;
    unsigned char buffer[1024];
    size_t bytesRead;

    file = fopen("example.bin", "rb");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    bytesRead = fread(buffer, 1, sizeof(buffer), file);
    if (bytesRead > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
    }

    fclose(file);
}

int main() {
    // 示例调用
    writeFile();
    readFile();
    appendFile();
    readBinaryFile();
    return 0;
}    
