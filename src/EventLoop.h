/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_EVENTLOOP_H
#define PROJECT_EVENTLOOP_H

#include <map>
#include <vector>
#include <memory>
#include <cerrno>
#include <pthread.h>
#include <functional>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "Mutex.h"
#include "Channel.h"
#include "EpollPoller.h"
class EventLoop {
public:
    typedef std::function<void()> Functor;

    EventLoop();
    EventLoop(EventLoop &) = delete;
    ~EventLoop();

    void loop();
    void runInLoop(const Functor func);
    void queueInLoop(const Functor func);

    void quit();
    void wakeup();
    void handleRead();
    void doPendingFunctors();
    /* Channel modified in EpollPoller finally which own them */
    bool hasChannel(Channel *channel);
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);

    bool isInLoopThread() { return threadId_ == gettid(); }
private:
    bool        quit_;
    bool        looping_;
    bool        eventHandling_;
    bool        callingPendingFunctor_;
    const pid_t threadId_;
    int         wakeupFd;
    MutexLock   mutex_;
    Channel     *currentActiveChannel;

    std::shared_ptr<Channel>        wakeupChannel_;
    std::shared_ptr<EpollPoller>    poller_;

    std::vector<Channel *>              activeChannels_;
    std::vector<std::function<void()>>  pendingFunctor_;
};

#endif //PROJECT_EVENTLOOP_H
