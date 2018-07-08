/*
 * Created by Jemmy on 2018/7/8.
 *
 */

EpollPoller::EpollPoller(EventLoop *loop)
        :  ownerLoop_(loop),
           eventVec(CInitEventVecSize),
           epollFd(epoll_create1(EPOLL_CLOEXEC))
{
    assert(epollFd > 0);
}

#include "EpollPoller.h"

void EpollPoller::poll(int ms, std::vector<Channel *> activeChannels) {
    int num = epoll_wait(epollFd, &*eventVec.begin(),
            static_cast<int>(eventVec.size()), ms);
    if (num > 0) {
        fillActiveChannel(num, activeChannels);
        if (static_cast<size_t >(num) == eventVec.size()) {
            eventVec.reserve(eventVec.size() * 2);
        }
    } else if (num == 0) {

    } else {
        printf("epoll_wait Error: %s\n", strerror(errno));
    }
}

void EpollPoller::fillActiveChannel(int num, std::vector<Channel *> activeChannel) {
    assert(static_cast<size_t >(num) < eventVec.size());
    for (int i = 0; i < num; ++i) {
        Channel *ch = static_cast<Channel *>(eventVec[i].data.ptr);
        int fd = ch->getFd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == ch);

        ch->setEvents(eventVec[i].events);
        activeChannel.push_back(ch);
    }
}


void EpollPoller::updateChannel(Channel *channel) {
    int index = channel->getIndex();
    int fd = channel->getFd();
    if (index == CNew || index == CDeleted) {
        if (index == CNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setEvents(CAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] = channel);
        assert(channel->getIndex() == CAdded);
        if (channel->isNonEvent()) {
            update(EPOLL_CTL_DEL, channel);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

/* not remove Channel really, just mark it as CNew */
void EpollPoller::removeChannel(Channel *channel) {
    int fd = channel->getFd();
    int index = channel->getIndex();
    assert(ownerLoop_->isInLoopThread());
    assert(channel->isNonEvent());
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == CAdded || index == CDeleted);
    if (index == CAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(CNew);
}

void EpollPoller::update(int op, Channel *channel) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = channel->getEvents();
    ev.data.ptr = channel;
    int fd = channel->getFd();
    if (epoll_ctl(epollFd, op, fd, &ev) < 0) {
        printf("update fd[%d] Error: %s\n", fd, strerror(errno));
    }
}

bool EpollPoller::hasChannel(Channel *channel) {
    std::map<int, Channel *>::const_iterator it = channels_.find(channel->getFd());
    return (it != channels_.end() && it->second == channel);
}
