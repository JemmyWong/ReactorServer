/*
 * Created by Jemmy on 2018/7/14.
 *
 */

#ifndef PROJECT_SOCKET_H
#define PROJECT_SOCKET_H

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

class Socket {
public:
    Socket(int fd);
    ~Socket();

    int getFd() { return sockFd_; }

    /* disable/enable Nagle's algorithm */
    void setTcpNODelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    void shutdown(int how);
private:
    const int sockFd_;
};

#endif //PROJECT_SOCKET_H
