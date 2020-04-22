/*******************************************************************************
 *      file name: va_list.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/02/03-16:19:34                              
 *  modified time: 2020/02/03-16:19:34                              
 *******************************************************************************/
#include <stdio.h>
#include <stdarg.h>

int myprintf(char *format, ...)
{
    va_list ap;
    char s[1000];
    int i;

    va_start(ap, format);
    i = vsprintf(s, format, ap);
    //api_putstr0(s);
    va_end(ap);
    return i;
}
int main()
{}
