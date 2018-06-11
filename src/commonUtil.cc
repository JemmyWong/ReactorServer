//
// Created by Jemmy on 2018/6/10.
//

#include "commonUtil.h"

/* daemon process */
void daemon() {
    int pid = fork();
    if (pid < 0) {
        printf("daemon fork error: %s\n", strerror(errno));
    } else if (pid > 0) {
        exit(0);
    }

    umask(0);
    setsid();

    int fd = open("/dev/null", O_RDWR, 0);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    if (fd > 2) {
        close(fd);
    }
}

/* set non-blocking socket */
int setNonBlock(int fd) {
    int oldFlags = fcntl(fd, F_GETFL);
    int newFlags = oldFlags | O_NONBLOCK;
    fcntl(fd, F_SETFL, newFlags);

    return oldFlags;
}

void addFd(int epollFd, int fd, bool oneShot) {
    struct epoll_event ee;
    ee.data.fd = fd;
    ee.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (oneShot) ee.events |= EPOLLONESHOT;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ee);
    setNonBlock(fd);
}

void removeFd(int epollFd, int fd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void modFd(int epollFd, int fd, int ev) {
    struct epoll_event ee;
    ee.data.fd = fd;
    ee.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ee);
}
