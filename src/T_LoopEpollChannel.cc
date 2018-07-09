//
// Created by by on 2018/7/9.
//
#include "Channel.h"
#include "EventLoop.h"
#include "EpollPoller.h"

#include <sys/timerfd.h>
#include <iostream>
#include <functional>

using namespace std;

EventLoop *gLoop = nullptr;

void timeout() {
    printf("Timeout....\n");
    gLoop->quit();
}

int main() {
    EventLoop loop;
    gLoop = &loop;

    int timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerFd);
    channel.setReadCB(std::bind(timeout));
    channel.enableRead();

    struct itimerspec howlong;
    memset(&howlong, 0, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    timerfd_settime(timerFd, 0, &howlong, NULL);

    loop.loop();

    close(timerFd);
}
