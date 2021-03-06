/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_EVENTLOOPTHREAD_H
#define PROJECT_EVENTLOOPTHREAD_H

#include <string>
#include <functional>

#include "Slog.h"
#include "Thread.h"
#include "EventLoop.h"

class EventLoopThread {
public:
    typedef std::function<void(EventLoop *)> ThreadInitCB;

    explicit EventLoopThread(const ThreadInitCB &cb = ThreadInitCB(), const std::string &name = std::string());
    ~EventLoopThread();
    EventLoop *startLoop();

private:
    void threadFunc();

    EventLoop       *loop_;
    Thread          thread_;
    MutexLock       mutex_;
    Condition       cond_;
    bool            exiting_;
    ThreadInitCB    threadCB_; /* called when thread create */
};

#endif //PROJECT_EVENTLOOPTHREAD_H
