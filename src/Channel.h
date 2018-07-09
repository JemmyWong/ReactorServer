/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_CHANNEL_H
#define PROJECT_CHANNEL_H

#include "EventLoop.h"

class EventLoop;

class Channel {
public:
    typedef std::function<void()> Functor;

    Channel(EventLoop *, int);
//    Channel(Channel &) = delete; /* non-copyable */
    ~Channel();

    /* Tie this channel to the owner object managed by shared_ptr,
     * prevent the owner object being destroyed in handleEvent. */
    void tie(std::shared_ptr<void> &);
    EventLoop *ownerLoop() { return loop_; }

    void handleEvent();
    void setReadCB(const Functor &func)  { readCB_ = func; printf("...setReadCB()");}
    void setWriteCB(const Functor &func) { writeCB_ = func; }
    void setErrorCB(const Functor &func) { errorCB_ = func; }

    int getFd() {return fd_; }
    int getIndex() { return index_; }
    int getEvents() { return events_; }

    int setIndex(int idx) { index_ = idx; }
    int setEvents(int e) { events_ = e; }

    void remove();
    void update();
    void enableRead()   { events_ |= CReadEvent; printf("...enableRead()\n");update(); }
    void disableRead()  { events_ &= ~CReadEvent; update(); }
    void enableWrite()  { events_ |= CWriteEvent; update(); }
    void disableWrite() { events_ &= CWriteEvent; update(); }
    void disableAll()   { events_ = CNoneEvent; update(); }
    bool isReading()    { return events_ & CReadEvent; }
    bool isWriting()    { return events_ & CWriteEvent; }
    bool isNonEvent()   { return events_ == CNoneEvent; }

private:
    void handleEventWithGuard();

    EventLoop *loop_;
    const int fd_;
    int index_;     /* status: CNew, CAdded, CDeleted */
    int events_;
    int rcvEvents_;
    std::weak_ptr<void> tie_;

    bool tied_;
    bool addToLoop_;
    bool eventHandling_;

    Functor readCB_;
    Functor writeCB_;
    Functor errorCB_;

    static const int CNoneEvent;
    static const int CReadEvent;
    static const int CWriteEvent;
};

#endif //PROJECT_CHANNEL_H
