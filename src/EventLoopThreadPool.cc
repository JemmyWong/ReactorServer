/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(const EventLoop *loop, std::string name = std::string())
        : baseLoop_(loop),
          name_(name),
          next_(0),
          numThreads_(0),
          started_(false)
{
}

void EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCB &cb) {
    assert(!started_);
    assert(baseLoop_->isInLoopThread());
    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s-%d", name_, i);
        EventLoopThreadPtr t = std::make_shrared<EventLoopThread>(cb, buf);
    }
}