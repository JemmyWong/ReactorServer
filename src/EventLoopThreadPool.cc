/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop, const std::string &name)
        : baseLoop_(loop),
          name_(name),
          next_(0),
          numThreads_(0),
          started_(false)
{ }

EventLoop *EventLoopThreadPool::getNextLoop() {
    assert(baseLoop_->isInLoopThread());
    assert(started_);
    EventLoop *loop = baseLoop_;
    /* round-robin */
    if (!loops_.empty()) {
        loop = loops_[next_++];
        next_ %= loops_.size();
    }
    return loop;
}

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCB &cb) {
    assert(!started_);
    assert(baseLoop_->isInLoopThread());
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s-%d", name_, i);
        EventLoopThreadPtr t = std::make_shared<EventLoopThread>(cb, std::string(buf));
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}