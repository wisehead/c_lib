/*******************************************************************************
 *      file name: bigend.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/01/29-20:46:33                              
 *  modified time: 2020/01/29-20:46:33                              
 *******************************************************************************/
#include <stdio.h>

void main()
{
  int a=0x01020304;
  
  unsigned char* a1=(unsigned char*)(&a);
  unsigned char* a2=(a1+1);
  unsigned char* a3=(a1+2);
  unsigned char* a4=(a1+3);
  
  
  printf("a:%x\n",  a);
  printf("a1:%d\n", *a1);
  printf("a2:%d\n", *a2);
  printf("a3:%d\n", *a3);
  printf("a4:%d\n", *a4);
  
  return ;
}

