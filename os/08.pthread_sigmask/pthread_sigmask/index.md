
# (22条消息) pthread_sigmask_dashoumeixi的博客-CSDN博客_pthread_sigmask sig_block

给群里的. apue和man里都有的; 对于线程信号,你应该忘记signal / sigaction ,他们只为单进程单线程设计

pthread\_sigmask 跟 sigprocmask 类似;

sigprocmask 只能用于单进程单线程; fork的子进程拥有一份屏蔽信号拷贝;

pthread\_sigmask 用于[多线程](https://so.csdn.net/so/search?q=%E5%A4%9A%E7%BA%BF%E7%A8%8B&spm=1001.2101.3001.7020) ; 新线程拥有一份pthread\_create那个线程的屏蔽信号拷贝;

对于线程信号的处理 ， 最好还是用一个线程来统一处理比较, 否则太乱啦!

关于线程安全和可重入, 完全2个概念: [线程安全 可重入](https://blog.csdn.net/dashoumeixi/article/details/95041967)

在多线程中处理信号 就别用sigaction / signal 了, 不然还得考虑重入,线程安全等问题,太麻烦;,而且这2个函数都是进程范围的;

下面所有代码中用到了sigwait \[ 信号同步化, 不再调用注册函数了\]

你也可以用sigwaitinfo , 用法说明 :  [sigwaitinfo](https://blog.csdn.net/dashoumeixi/article/details/95113254)

1.关于sigwait / sigwaitinfo 这种同步化的方式一般要先block信号, 否则还检查啥pendding信号呢 ,不同的linux版本有不同的处理方式, 具体情况还是看man把

2.如果看了下面代码还是不了解同步化信号的方式  ， 可以参考windows的消息机制 ;

第一个例子  替换 sigprocmask:

```cpp
//gcc 别忘了 -lpthread
void handler(int s){
    printf("handler :%d\n" ,s);
}
 
int main(int argc, char**argv)
{
    signal(SIGINT , handler);
    sigset_t mask;
    sigaddset(&mask , SIGINT);
    pthread_sigmask(SIG_BLOCK,&mask, NULL); //替换sigprocmask
 
    //用ctrl+\ 终止
    while(1)
        pause();
    return 0;
}
```

第2个例子 , 让子线程处理信号;  ctrl+c 触发他 , 此信号发送给进程的

如果把下面注释的代码打开 , 则有2个线程都能处理信号, 到底用哪个线程 ?  一般情况下主线程

为什么是主线程 ? linux下的线程由进程实现

```cpp
#include "util.h"
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
 
static void * t1(void *arg){
    sigset_t * mask = (sigset_t *)arg;
    int sig = 0;
 
    while(1){
        if ( sigwait(mask,&sig)  != 0 ){
            perror(" thread sigwait :");
            continue;
        }
        printf("thread got signal : %d\n" , sig);
    }
}
 
int main(int argc, char**argv)
{
    sigset_t mask;
    sigaddset(&mask , SIGINT);
    pthread_sigmask(SIG_BLOCK,&mask, NULL);
    pthread_t t;
    pthread_create(&t,0,t1,&mask);
 
    int sig = 0;
    
    while(1) pause();
 
    /*
    while(1){
        if ( sigwait(&mask,&sig)  != 0 ){
            perror("sigwait :");
            continue;
        }
        printf(" ! main got signal : %d\n" , sig);
    }

    */
 
    return 0;
}
```

第3个例子 ，主线程调用alarm , 让子线程去处理 , ctrl+c 结束;

 屏蔽了所有信号,不再一个个加了,麻烦

```cpp
 
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
volatile sig_atomic_t goon = 1;
 
static void * t1(void *arg){
    sigset_t * mask = (sigset_t *)arg;
    int sig = 0;
 
    while(1){
        if ( sigwait(mask,&sig)  != 0 ){
            perror(" thread sigwait :");
            continue;
        }
        printf("thread got signal : %d\n" , sig);
        if(SIGINT == sig){
            goon = 0;
            break;
        }
    }
}
 
int main(int argc, char**argv)
{
    sigset_t mask;
    sigfillset(&mask); //屏蔽所有信号
    pthread_sigmask(SIG_BLOCK,&mask, NULL);
    pthread_t t;
    pthread_create(&t,0,t1,&mask);
 
    int sig = 0;
    while(goon){
        alarm(1);
        sleep(1);
    }
 
    pthread_join(t,0);
    puts("end");
    return 0;
}
```

第4个例子 , 一共3个线程, 主线程跟上面一样,每秒调用alarm , 一个专门处理信号的线程 , 还有一个工作线程;

```cpp
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
volatile sig_atomic_t goon = 1;
 
static void * worker (void *arg){
    while(goon){
        printf("worker is running , tid:%ld\n" , pthread_self());
        sleep(5);
    }
    puts("worker is done");
}
 
static void * sig_handler_thread(void *arg){
    sigset_t * mask = (sigset_t *)arg;
    int sig = 0;
    pthread_t tid = pthread_self();
    while(1){
        if ( sigwait(mask,&sig)  != 0 ){
            printf("sigwait error : %s\n" , strerror(errno));
            continue;
        }
        printf("thread :%ld got signal : %d\n" , tid,sig);
        if(SIGINT == sig){
            goon = 0;
            break;
        }
    }
}
 
int main(int argc, char**argv)
{
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK,&mask, NULL);
    pthread_t tid1, tid2;
    pthread_create(&tid1,0,sig_handler_thread,&mask);
    pthread_create(&tid2,0,worker,NULL);
 
    int sig = 0;
    while(goon){
        alarm(1);
        sleep(1);
    }
 
    pthread_join(tid2,0);
    pthread_join(tid1, NULL);
    puts("end");
    return 0;
}
```