/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "Acceptor.h"


int createNonBlockingFd(const std::string addr) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    assert(fd > 0);

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(9000);
    local.sin_addr.s_addr = INADDR_ANY;

    int n = bind(fd, reinterpret_cast<struct sockaddr *>(&local), sizeof(local));
    if (n < 0) {
        printf("bind error: %s\n", strerror(errno));
    }
    assert(n >= 0);

    listen(fd, 100);

    return fd;
}

Acceptor::Acceptor(EventLoop *loop, const std::string &addr)
        : sockFd_(createNonBlockingFd(addr)),
          loop_(loop),
          acceptChannel_(loop_, sockFd_),
          listening_(true)
{
    printf("...Acceptor()\n");
    acceptChannel_.setReadCB(std::bind(&Acceptor::handleRead, this));
    acceptChannel_.enableRead();
}

Acceptor::~Acceptor() {

}

void Acceptor::handleRead() {
    assert(loop_->isInLoopThread());
    struct sockaddr_in peerAddr;
    socklen_t len = sizeof(peerAddr);
    int newFd = accept(sockFd_, reinterpret_cast<struct sockaddr *>(&peerAddr), &len);
    if (newFd > 0) {
        if (newConnectionCB_) {
            newConnectionCB_(newFd, &peerAddr);
        }
    } else {
        printf("accept() failed...\n");
    }


}


