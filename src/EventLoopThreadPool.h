/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_EVENTLOOPTHREADPOOL_H
#define PROJECT_EVENTLOOPTHREADPOOL_H

#include "EventLoop.h"
#include "EventLoopThread.h"

class EventLoopThreadPool {
public:
    typedef std::function<void(EventLoop *)> ThreadInitCB;
    typedef std::shared_ptr<EventLoopThread> EventLoopThreadPtr;

    EventLoopThreadPool(EventLoop *loop, const std::string &name);

    /* loops are stack variable no need to delete */
    ~EventLoopThreadPool() = default;

    void setNumThreads(int num) { numThreads_ = num; }

    void start(const ThreadInitCB &cb = ThreadInitCB());

    EventLoop *getNextLoop();

    bool started() { return started_; }
private:
    EventLoop                   *baseLoop_;
    std::string                 name_;
    int                         next_;
    int                         numThreads_;
    bool                        started_;
    std::vector<EventLoopThreadPtr> threads_;
    std::vector<EventLoop *>    loops_;         /* stack variables */
};

#endif //PROJECT_EVENTLOOPTHREADPOOL_H
