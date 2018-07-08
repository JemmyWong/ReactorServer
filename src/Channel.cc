/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "Channel.h"

const int Channel::CNoneEvent = 0;
const int Channel::CReadEvent = POLLIN | POLLPRI;
const int Channel::CWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : loop_(loop),
          fd_(fd),
          index_(-1),
          events_(0),
          rcvEvents_(0),
          tied_(false),
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
    eventHandling_ = true;
    /*if ((rcvEvents_ & POLLHUP) && !(rcvEvents_ & POLLIN)) {
        if (logHup_) {
            LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if (closeCallback_) closeCallback_();
    }*/

    if (rcvEvents_ & POLLNVAL) {
        std::cout << "fd = " << fd_ << " Channel::handle_event() POLLNVAL" << std::endl;
    }

    if (rcvEvents_ & (POLLERR | POLLNVAL)) {
        if (errorCB_) errorCB_();
    }
    if (rcvEvents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCB_) readCB_();
    }
    if (rcvEvents_ & POLLOUT) {
        if (writeCB_) writeCB_();
    }
    eventHandling_ = false;
}

void Channel::tie(std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}