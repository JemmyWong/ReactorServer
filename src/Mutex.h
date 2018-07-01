//
// Created by Jemmy on 2018/7/1.
//

#ifndef PROJECT_MUTEX_H
#define PROJECT_MUTEX_H

#include <iostream>
#include <cerrno>
#include <cstring>
#include <cassert>

#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

class MutexLock {
public:
    explicit MutexLock();
    ~MutexLock() ;
    void lock();
    void unlock();
    void assertLocked();
    bool isLockedByThisThread();
    pthread_mutex_t getPthreadMutex();
private:
    pthread_t holder_;
    pthread_mutex_t mutex_;
};

class MutexLockGuard {
public:
    explicit MutexLockGuard(MutexLock& mutex);
    ~MutexLockGuard();
private:
    MutexLock &mutex_;
};

//#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")

class Condition {
public:
    explicit Condition(MutexLock &mutex);
    ~Condition();
    void wait();
    void notify();
    void notifyAll();
private:
    MutexLock &mutex_;
    pthread_cond_t pcond_;
};

class CountDownLatch {
public:
    explicit CountDownLatch(int cnt);
    void wait();
    void countDown();
    int getCount();
private:
    MutexLock mutex_;
    Condition cond_;
    int count_;
}

#endif //PROJECT_MUTEX_H
