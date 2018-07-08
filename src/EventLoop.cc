/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "EventLoop.h"

const int CPollTimeMs = 10000;

int createEventfd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd > 0);
    return fd;
}

EventLoop::EventLoop()
        : quit_(false),
          looping_(false),
          eventHandling_(false),
          callingPendingFunctor_(false),
          threadId_(gettid()),
          wakeupFd(createEventfd()),
          currentActiveChannel(nullptr),
          wakeupChannel_(new Channel(this, wakeupFd)),
          poller_(std::make_shared<EpollPoller>(this))
{
    wakeupChannel_->setReadCB(std::bind(EventLoop::handleRead));
    wakeupChannel_->enableRead();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd);
}

void EventLoop::loop() {
    assert(!looping_);
    assert(isInLoopThread());
    looping_= true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(CPollTimeMs, activeChannels_);

        eventHandling_ = true;
        for (std::vector<Channel *>::iterator it = activeChannels_.begin();
                it != activeChannels_.end(); ++it) {
            currentActiveChannel = *it;
            currentActiveChannel->handleEvent();
        }
        currentActiveChannel = NULL;
        eventHandling_ = false;

        doPendingFunctors();
    }

    looping_ = false;
    quit_ = true;
}

void EventLoop::doPendingFunctors() {
    std::vector<std::function<void()>> functors;
    {
        MutexLockGuard lock(mutex_);
        pendingFunctor_.swap(functors);
    }

    callingPendingFunctor_ = true;
    for (auto it = functors.begin(); it != functors.end(); ++it) {
        (*it)();
    }
    callingPendingFunctor_ = false;
}

void EventLoop::runInLoop(const EventLoop::Functor func) {
    if (isInLoopThread()) {
        func();
    } else {
        queueInLoop(func);
    }
}

void EventLoop::queueInLoop(const EventLoop::Functor func) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctor_.push_back(func);
    }
    if (!isInLoopThread() || callingPendingFunctor_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    int one = 1;
    ssize_t n = write(wakeupFd, &one, sizeof(one));
}

bool EventLoop::hasChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    poller_->hasChannel(channel);
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assert(isInLoopThread());
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assert(isInLoopThread());
    if (eventHandling_) {
        assert(currentActiveChannel == channel
        || std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

void EventLoop::handleRead() {
    int one = 1;
    ssize_t n = read(wakeupFd, &one, sizeof(one));
}

void EventLoop::quit() {
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!isInLoopThread()) {
        wakeup();
    }
}