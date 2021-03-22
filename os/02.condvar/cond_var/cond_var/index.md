---
title: 条件变量实例 - TheBug - 博客园
category: default
tags: 
  - www.cnblogs.com
created_at: 2020-11-03 10:25:48
original_url: https://www.cnblogs.com/jacksu-tencent/p/3366238.html
---


# 条件变量实例 - TheBug - 博客园

   条件变量(Condtion Variable)是在多线程程序中用来实现“等待->唤醒”逻辑常用的方法。举个简单的例子，应用程序A中包含两个线程t1和t2。t1需要在bool变量test\_cond为true时才能继续执行，而test\_cond的值是由t2来改变的，这种情况下，如何来写程序呢？可供选择的方案有两种：

*   第一种是t1定时的去轮询变量test\_cond，如果test\_cond为false，则继续休眠；如果test\_cond为true，则开始执行。
*   第二种就是上面提到的条件变量，t1在test\_cond为false时调用cond\_wait进行等待，t2在改变test\_cond的值后，调用cond\_signal，唤醒在等待中的t1，告诉t1 test\_cond的值变了，这样t1便可继续往下执行。

      很明显，上面两种方案中，第二种方案是比较优的。在第一种方案中，在每次轮询时，如果t1休眠的时间比较短，会导致cpu浪费很厉害；如果t1休眠的时间比较长，又会导致应用逻辑处理不够及时，致使应用程序性能下降。第二种方案就是为了解决轮询的弊端而生的。然而条件变量在使用的过程中，比较容易出错，如何用得不正确的话，会适得其反的，接下来，我将详细分析如何来使用条件变量，希望能够给在使用条件变量过程中遇到问题的朋友有所帮助。       在开始介绍之前，需要说明一下，在接下来的介绍中，需要用到互斥锁和条件变量相关的内容，在这里我以linux下的pthread\_mutex\_t为互斥锁类型，pthread\_cond\_t为条件变量类型来进行介绍，对pthread不熟的朋友，可以参考一下linux下的manual。       1. 下面是把刚开始举的例子翻译后的程序：

<table><tbody><tr><td class="line_numbers"><div><pre data-mx-wc-processed=""><code class="language-plain">1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21</code></pre></div></td><td class="code"><div><pre data-mx-wc-processed=""><code class="language-plain">pthread_mutex_t mutex;  ///&lt; 互斥锁
pthread_cond_t  cond;   ///&lt; 条件变量
bool test_cond = false;
/// TODO 初始化mutex和cond
 
/// thread 1:
pthread_mutex_lock(&amp;mutex);            ///&lt; 1
while (!test_cond)
//必须while，wait 返回不代表条件成立，可能会有别的线程抢先又使得条件不成立了
{
    pthread_cond_wait(&amp;cond, &amp;mutex);  ///&lt; 2,3
}
pthread_mutex_unlock(&amp;mutex);          ///&lt; 4
RunThread1Func();
 
/// thread 2:
pthread_mutex_lock(&amp;mutex);            ///&lt; 5
test_cond = true;
pthread_cond_signal(&amp;cond);
pthread_mutex_unlock(&amp;mutex);          ///&lt; 6
 
/// TODO 销毁mutex和cond</code></pre></div></td></tr></tbody></table>

      通过上面的例子，下面我来介绍一下条件变量在使用过程中需要注意的几点（也是比较容易出错的）：       （1）条件变量的使用过程中，最为关键的一点是互斥锁的使用。细心的朋友应该发现了，我在上面的例子中标了1、2、3、4、5、6个标号。在这里1、4、5、6都是正常的lock/unlock，2、3是需要特别说明的。2是进入pthread\_cond\_wait后的，pthread\_cond\_wait调的pthread\_mutex\_unlock，这样做的目的是为了保证在thread1阻塞wait后，thread2获取同一把锁mutex的时候，能够正常获取（即5，6）。3是thread1被唤醒后，要退出pthead\_cond\_wait之前，pthread\_cond\_wait调的pthread\_mutex\_lock，这样做的目的是为了把mutex的控制权还给调用pthread\_cond\_wait的线程（即thread1）。整理一下基本的时序为：

<table><tbody><tr><td class="line_numbers"><div><pre data-mx-wc-processed=""><code class="language-plain">1
2
3</code></pre></div></td><td class="code"><div><pre data-mx-wc-processed=""><code class="language-plain">thread 1 lock-&gt;thread 1 wait-&gt; thread 1 unlock(in wait)
-&gt;thread 2 lock-&gt;thread 2 signal-&gt;thread 2 unlock
-&gt;thread 1 lock(in wait)-&gt;thread 1 unlock</code></pre></div></td></tr></tbody></table>

      （2）条件变量使用的过程中，通常会加一个bool或者int的值test\_cond来配合使用。这里需要注意的一点是一定要在signal之前来改变test\_cond，这样才能保证wait的线程被唤醒后，能够取到正确的test\_cond的值，否则后果是不可预测的。

参考：

[http://www.wuzesheng.com/?p=1668](http://www.wuzesheng.com/?p=1668)

---------------------------------------------------


原网址: [访问](https://www.cnblogs.com/jacksu-tencent/p/3366238.html)

创建于: 2020-11-03 10:25:48

目录: default

标签: `www.cnblogs.com`

