/*******************************************************************************
 *      file name: inversion.c                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/01/28-18:12:22                              
 *  modified time: 2020/01/28-18:12:22                              
 *******************************************************************************/
#include <stdio.h>
#define EFLAGS_AC_BIT           0x00040000
int main()
{
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    /* 确认CPU是386还是486以上的 */
    eflg = 0xff;
    eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
    if ((eflg & EFLAGS_AC_BIT) != 0) {
        /* 如果是386，即使设定AC=1，AC的值还会自动回到0 */
        flg486 = 1;
    }
	printf("eflg:%u.\n", eflg);

    eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	printf("eflg:%u.\n", eflg);
	return 0;
}
