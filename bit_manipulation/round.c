/*******************************************************************************
 *      file name: round.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/01/28-18:32:05                              
 *  modified time: 2020/01/28-18:32:05                              
 *******************************************************************************/
#include <stdio.h>

int main()
{
    unsigned int a, old, size = 0x00100100;
	printf("size:%x.\n", size);
	old = size;
	//lower bound
    size = size & 0xfffff000;
	printf("lower bound size:%x.\n", size);
	//upper bound
	size = old;
    size = (size + 0xfff) & 0xfffff000;
	printf("upper bound size:%x.\n", size);
    return a;
}
