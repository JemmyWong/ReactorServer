/*
 * Created by Jemmy on 2018/7/9.
 *
 */
//
// Created by by on 2018/7/9.
//
#include "Channel.h"
#include "EventLoop.h"
#include "EpollPoller.h"
#include "Acceptor.h"

#include <sys/timerfd.h>
#include <iostream>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

using namespace std;

EventLoop *gLoop = nullptr;

void newConnectionCB(int fd, const struct sockaddr_in * remote) {
    printf("newConnectionCB()....fd{%d}, ip: %s, port: %d\n", fd, inet_ntoa(remote->sin_addr), ntohs(remote->sin_port));
}

int main() {
    EventLoop loop;
    gLoop = &loop;
    string addr = "172";
    Acceptor acceptor(&loop, addr);
    acceptor.setNewConnectionCB(newConnectionCB);

    loop.loop();

}


