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

void daemon();

int setNonBlock(int fd);

void addFd(int epollFd, int fd, bool oneShot);
void removeFd(int epollFd, int fd);
void modFd(int epollFd, int fd, int ev);

#endif //PROJECT_COMMONUTIL_H
