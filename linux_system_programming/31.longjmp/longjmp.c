/*******************************************************************************
 *      file name: longjmp.c                                               
 *         author: hui chen. (c) 25                             
 *           mail: alex.chenhui@gmail.com                                        
 *   created time: 25/11/11-11:11                                    
 * modified time: 25/11/11-11:11                                    
 *******************************************************************************/
#include <stdio.h>
#include <setjmp.h>

jmp_buf env; // 全局的跳转缓冲区

void func() {
    printf("进入 func()\n");
    // 跳回 setjmp 保存的位置，返回值为 1
    longjmp(env, 1); 
    printf("func() 结束（不会执行）\n"); // 跳转后此句被跳过
}

int main() {
    // 保存当前执行环境到 env，首次返回 0
    int ret = setjmp(env);
    if (ret == 0) {
        printf("首次执行 setjmp，调用 func()\n");
        func(); // 调用函数
    } else {
        printf("从 longjmp 跳转回来，ret = %d\n", ret);
    }
    return 0;
}
