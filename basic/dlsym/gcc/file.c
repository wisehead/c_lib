/*******************************************************************************
 *      file name: file.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/04/22-10:27:02                              
 *  modified time: 2020/04/22-10:27:02                              
 *******************************************************************************/
#include <stdio.h>
// file : add.c
int add(int a, int b) { return a+b; };

// cmd: gcc -fPIC -shared -o libadd.so add.c
// 编译生成动态库文件
