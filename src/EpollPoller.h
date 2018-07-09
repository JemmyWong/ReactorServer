/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_EPOLLPOLLER_H
#define PROJECT_EPOLLPOLLER_H

#include "Channel.h"
#include <map>
#include <vector>

class EventLoop;
class Channel;

class EpollPoller {
public:
    EpollPoller(EventLoop *);
    EpollPoller(EpollPoller &) = delete;
    ~EpollPoller();

    void poll(int ms, std::vector<Channel *> *activeChannels);
    void update(int op, Channel *channel);
    bool hasChannel(Channel *channel);
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
private:
    typedef std::map<int, Channel *>    ChannelMap;

    void fillActiveChannel(int num, std::vector<Channel *> *activeChannel);

    EventLoop                           *ownerLoop_;
    int                                 epollFd_;
    ChannelMap                          channels_;

    std::vector<struct epoll_event> eventVec_;

    static const int CNew = -1;
    static const int CAdded = 1;
    static const int CDeleted = 2;
    static const int CInitEventVecSize = 16;
};

#endif //PROJECT_EPOLLPOLLER_H
