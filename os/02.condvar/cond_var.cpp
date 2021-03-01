/*******************************************************************************
 *      file name: cond_var.cpp                                               
 *         author: Hui Chen. (c) 2020                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2020/11/03-10:26:33                              
 *  modified time: 2020/11/03-10:26:33                              
 *******************************************************************************/
#include <iostream>
using namespace std;
pthread_mutex_t mutex;  ///< 互斥锁
pthread_cond_t  cond;   ///< 条件变量
bool test_cond = false;
/// TODO 初始化mutex和cond
 
/// thread 1:
pthread_mutex_lock(&mutex);            ///< 1
while (!test_cond)
//必须while，wait 返回不代表条件成立，可能会有别的线程抢先又使得条件不成立了
{
    pthread_cond_wait(&cond, &mutex);  ///< 2,3
}
pthread_mutex_unlock(&mutex);          ///< 4
RunThread1Func();
 
/// thread 2:
pthread_mutex_lock(&mutex);            ///< 5
test_cond = true;
pthread_cond_signal(&cond);
pthread_mutex_unlock(&mutex);          ///< 6
 
/// TODO 销毁mutex和cond
int main()
{}
