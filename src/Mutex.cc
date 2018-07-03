//
// Created by Jemmy on 2018/7/1.
//

#include "Mutex.h"
/*************************** MutexLock *********************************/
MutexLock::MutexLock(): holder_(0) {
    pthread_mutex_init(&mutex_, NULL);
}

MutexLock::~MutexLock() {
    assert(holder_ == 0);
    pthread_mutex_destroy(&mutex_);
}

void MutexLock::lock() {
    pthread_mutex_lock(&mutex_); // order can't be changed
    holder_ = static_cast<pid_t>(::syscall(SYS_gettid));
}

void MutexLock::unlock() {
    holder_ = 0;                // order can't be changed
    pthread_mutex_unlock(&mutex_);
}

void MutexLock::assertLocked() {
    assert(isLockedByThisThread());
}

bool MutexLock::isLockedByThisThread() {
    return holder_ == static_cast<pid_t>(::syscall(SYS_gettid));;
}

pthread_mutex_t MutexLock:: getPthreadMutex() {
    return mutex_;
}

/*************************** MutexLockGuard *****************************/

MutexLockGuard::MutexLockGuard(MutexLock& mutex): mutex_(mutex) {
    mutex_.lock();
}

MutexLockGuard::~MutexLockGuard() {
    mutex_.unlock();
}

/*************************** Condition *********************************/

Condition::Condition(MutexLock &mutex): mutex_(mutex) {
    pthread_cond_init(&pcond_, NULL);
}

Condition::~Condition() {
    pthread_cond_destroy(&pcond);
}

void Condition:: wait() {
    pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
}

void Condition:: notify() {
    pthread_cond_signal(&pcond);
}

void Condition:: notifyAll() {
    pthred_cond_brodcast(&pcond);
}

/*************************** CountDownLatch ****************************/

CountDownLatch::CountDownLatch(int cnt):mutex_(), cond_(mutex_), count_(cnt) {}

void CountDownLatch::wait() {
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
        cond_.wait();
    }
}

void CountDownLatch::countDown() {
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0) {
        cond_.notifyAll();
    }
}

int CountDownLatch::getCount() {
    MutexLockGuard lock(mutex_);
    return count_;
}

