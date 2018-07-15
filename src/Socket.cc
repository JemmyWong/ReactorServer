/*
 * Created by Jemmy on 2018/7/14.
 *
 */

#include "Socket.h"

Socket::Socket(int fd) : sockFd_(fd) { }

Socket::~Socket() {
    ::close(sockFd_);
}

void Socket::shutdown(int how) {
    ::shutdown(sockFd_, how);
}

void Socket::setReusePort(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEPORT,
                 &opt, static_cast<socklen_t>(sizeof(opt)));
}

void Socket::setReuseAddr(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR,
                 &opt, static_cast<socklen_t>(sizeof(opt)));
}

void Socket::setTcpNODelay(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, IPPROTO_TCP, TCP_NODELAY,
                 &opt, static_cast<socklen_t>(sizeof(opt)));
}

void Socket::setKeepAlive(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, SOL_SOCKET, SO_KEEPALIVE,
                 &opt, static_cast<socklen_t>(sizeof(opt)));
}