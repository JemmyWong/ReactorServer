/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "Channel.h"

const int Channel::CNoneEvent = 0;
const int Channel::CReadEvent = EPOLLIN | EPOLLPRI | EPOLLHUP | EPOLLET;
const int Channel::CWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : loop_(loop),
          fd_(fd),
          index_(-1),
          events_(0),
          rcvEvents_(0),
          tied_(false),
          logHup_(true),
          addToLoop_(false),
          eventHandling_(false)
{ }

Channel::~Channel() {
    assert(!eventHandling_);
    assert(!addToLoop_);
    if (loop_->isInLoopThread()) {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::update() {
    addToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    assert(isNonEvent());
    addToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent() {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handleEventWithGuard();
        }
    } else {
        handleEventWithGuard();
    }
}

void Channel::handleEventWithGuard() {
    printf("%s->%s\n", __FILE__, __func__);
    eventHandling_ = true;
    if ((rcvEvents_ & EPOLLHUP) /*&& !(rcvEvents_ & EPOLLIN)*/) {
        if (logHup_) {
            printf("fd = %d, Channel::handle_event() EPOLLHUP\n", fd_);
        }
        if (closeCB_) closeCB_();
    }

    /*if (rcvEvents_ & EPOLLNVAL) {
        std::cout << "fd = " << fd_ << " Channel::handle_event() POLLNVAL" << std::endl;
    }*/

    if (rcvEvents_ & (EPOLLERR)) {
        if (errorCB_) errorCB_();
    }
    if (rcvEvents_ & (EPOLLIN | EPOLLPRI)) {
        if (readCB_) readCB_();
    }
    if (rcvEvents_ & EPOLLOUT) {
        if (writeCB_) writeCB_();
    }
    eventHandling_ = false;
}

void Channel::tie(const std::shared_ptr<void> &obj) {
    printf("%s->%s\n", __FILE__, __func__);
    tie_ = obj;
    tied_ = true;
}