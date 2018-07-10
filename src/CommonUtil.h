//
// Created by Jemmy on 2018/6/10.
//

#ifndef PROJECT_COMMONUTIL_H
#define PROJECT_COMMONUTIL_H

#include <iostream>
#include <cerrno>
#include <cstring>
#include <cassert>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h> /* umask*/
#include <sys/types.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <functional>
#include <memory>

#ifdef SYS_gettid
#define gettid syscall(SYS_gettid)
#else
#error "SYS_gettid unavailable on this system"
#endif

void daemon();

int setNonBlock(int fd);

void addFd(int epollFd, int fd, bool oneShot);
void removeFd(int epollFd, int fd);
void modFd(int epollFd, int fd, int ev);

class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectioinPtr;
typedef std::function<void(const TcpConnectioinPtr &)> CloseCB;
typedef std::function<void(const TcpConnectioinPtr &)> ConnectionCB;
typedef std::function<void(const TcpConnectioinPtr &)> WriteCompleteCB;
typedef std::function<void(const TcpConnectioinPtr &, const char *, int len)> MessageCB;


#endif //PROJECT_COMMONUTIL_H
