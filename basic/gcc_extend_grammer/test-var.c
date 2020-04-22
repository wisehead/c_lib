/*******************************************************************************
 *      file name: test-var.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/03/06-22:10:55                              
 *  modified time: 2020/03/06-22:10:55                              
 *******************************************************************************/
#include <stdio.h>
int var () {
	int local;
	static int a __attribute__ ((section("DUART_A")));
	static int req asm("ebx");
	return 0;
}
int main()
{}
