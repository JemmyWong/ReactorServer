/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "EventLoopThread.h"

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCB &cb, const std::string name = std::string())
        : loop_(NULL),
          thread_(std::bind(&EventLoopThread::threadFunc, this), name),
          mutex_(),
          cond_(mutex_),
          exiting_(false),
          threadCB_(cb)
{ }

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != NULL) {
        loop_->quit();
        thread_.join();
    }
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (threadCB_) {
        threadCB_(&loop);
    }
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop_->loop();
    loop_ = NULL;
}

EventLoop *EventLoopThread::startLoop() {
    assert(thread_.isStart());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL) {
            cond_.wait();
        }
    }
    return loop_;
}