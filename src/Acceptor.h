/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_ACCEPTOR_H
#define PROJECT_ACCEPTOR_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string>
#include <functional>
#include <cassert>

#include "Channel.h"
#include "EventLoop.h"

class Acceptor {
public:
    typedef std::function<void(int fd, const struct sockaddr_in *peerAddr)> NewConnectionCB;

    Acceptor(EventLoop *, const std::string &);
    ~Acceptor();

    void handleRead();

    bool isListening() { return listening_; }

    void setNewConnectionCB(const NewConnectionCB &cb) {
        newConnectionCB_ = cb;
    }
private:
    int             sockFd_;
    EventLoop       *loop_;
    Channel         acceptChannel_;
    NewConnectionCB newConnectionCB_;

    bool            listening_;
};

#endif //PROJECT_ACCEPTOR_H
