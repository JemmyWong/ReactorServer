/*
 * Created by Jemmy on 2018/7/8.
 *
 */
#include "Slog.h"
#include "EpollPoller.h"

EpollPoller::EpollPoller(EventLoop *loop)
        :  ownerLoop_(loop),
           eventVec_(CInitEventVecSize),
           epollFd_(epoll_create1(EPOLL_CLOEXEC))
{
    assert(epollFd_ > 0);
}

EpollPoller::~EpollPoller() {
    close(epollFd_);
}

void EpollPoller::poll(int ms, std::vector<Channel *> *activeChannels) {
    int num = epoll_wait(epollFd_,
            eventVec_.data(),/*&*eventVec_.begin()*/
            static_cast<int>(eventVec_.size()), ms);
    if (num > 0) {
        slog_info("[%d] ready resource", num);
        fillActiveChannel(num, activeChannels);
        /* The capacity has been used up, expand double */
        if (static_cast<size_t >(num) == eventVec_.size()) {
            eventVec_.reserve(eventVec_.size() * 2);
        }
    } else if (num == 0) {

    } else {
        printf("epoll_wait Error: %s\n", strerror(errno));
        slog_error("epoll_wait Error: %s", strerror(errno));
    }
}

void EpollPoller::fillActiveChannel(int num, std::vector<Channel *> *activeChannel) {
    assert(static_cast<size_t >(num) < eventVec_.size());
    for (int i = 0; i < num; ++i) {
        Channel *ch = static_cast<Channel *>(eventVec_[i].data.ptr);
        int fd = ch->getFd();
        printf("fd: %d\n", fd);
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == ch);

        ch->setEvents(eventVec_[i].events);
        activeChannel->push_back(ch);
    }
}


void EpollPoller::updateChannel(Channel *channel) {
    int index = channel->getStatus();
    int fd = channel->getFd();
    if (index == CNew || index == CDeleted) {
        if (index == CNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setStatus(CAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] = channel);
        assert(channel->getStatus() == CAdded);
        if (channel->isNonEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setStatus(CDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
    printf("ChannelMap:<%d,%d>\n", fd, channels_[fd]);
    slog_info("ChannelMap:<%d,%d>", fd, channels_[fd]);
}

void EpollPoller::removeChannel(Channel *channel) {
    int fd = channel->getFd();
    int index = channel->getStatus();
    assert(ownerLoop_->isInLoopThread());
    assert(channel->isNonEvent());
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == CAdded || index == CDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);
    if (index == CAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setStatus(CNew);
}

void EpollPoller::update(int op, Channel *channel) {
    printf("...updata(), fd[%d] op: %d\n", channel->getFd(), op);
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLET | channel->getEvents();
    ev.data.ptr = channel;
    int fd = channel->getFd();
    if (epoll_ctl(epollFd_, op, fd, &ev) < 0) {
        printf("update fd[%d] Error: %s\n", fd, strerror(errno));
    }
}

bool EpollPoller::hasChannel(Channel *channel) {
    std::map<int, Channel *>::const_iterator it = channels_.find(channel->getFd());
    return (it != channels_.end() && it->second == channel);
}
