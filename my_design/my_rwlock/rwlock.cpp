#include <cstddef>
#include <iostream>
#include <pthread.h>
using namespace std;

class Lock {
public:
  Lock() { pthread_rwlock_init(&rwlock, nullptr); }

  ~Lock() { pthread_rwlock_destroy(&rwlock); }

  void rlock() { pthread_rwlock_rdlock(&rwlock); }
  void unlock() { pthread_rwlock_unlock(&rwlock); }
  void wlock() { pthread_rwlock_wrlock(&rwlock); }

private:
  pthread_rwlock_t rwlock;
};

int main() {
  int x = 0;
  Lock lock;
  lock.wlock();
  x++;
  lock.unlock();
  lock.rlock();
  cout << "x:" << x << endl;
  lock.unlock();
}