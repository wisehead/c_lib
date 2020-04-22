/*******************************************************************************
 *      file name: inversion2.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/01/28-18:24:43                              
 *  modified time: 2020/01/28-18:24:43                              
 *******************************************************************************/
#include <stdio.h>

int main()
{
	unsigned int *p, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	*p = pat0;
	printf("*p:%x.\n", *p);
	*p ^= 0xffffffff;
	printf("*p:%x.\n", *p);
	*p ^= 0xffffffff;
	printf("*p:%x.\n", *p);
	return 0;
}
