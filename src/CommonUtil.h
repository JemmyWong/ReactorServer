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
class EventLoop;
class HttpRequest;
class HttpResponse;

typedef std::function<void(EventLoop *)> ThreadInitCB;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr &)> CloseCB;
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCB;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCB;
typedef std::function<void(const TcpConnectionPtr &, const char *, int len)> MessageCB;

typedef std::function<void(const HttpRequest &, HttpResponse *)> HttpCB;

/* http request method, only support GET */
enum Method {GET = 0, POST, HEAD, PUT, DELETE,
    TRACE, OPTIONS, CONNECT, PATCH};

/* state of the state machine */
enum CheckState {CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER, CHECK_STATE_CONTENT};

/* result of http request processed */
enum HttpCode { NO_REQUEST, GET_REQUEST, BAD_REQUEST,
    NO_RESOURCE, FORBIDDEN_REQUEST,_REQUEST, FILE_REQUEST,
    INTERNAL_ERROR, CLOSED_CONNECTION};

/* state of line read */
enum LineStatus {
    LINE_OK = 0,    /* complete request line */
    LINE_BAD,       /* incomplete request line */
    LINE_OPEN       /* line is reading */
};

enum Version {UNKNOW, HTTP10, HTTP11};

#endif //PROJECT_COMMONUTIL_H
