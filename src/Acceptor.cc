/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "Acceptor.h"


int createNonBlockingFd(const std::string addr) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    assert(fd > 0);
    int opt = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR,
                 &opt, static_cast<socklen_t>(sizeof(opt)));

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(atoi(addr.c_str()));
    local.sin_addr.s_addr = INADDR_ANY; /* inet_addr("0.0.0.0") */

    int n = bind(fd, reinterpret_cast<struct sockaddr *>(&local), sizeof(local));
    if (n < 0) {
        printf("bind error: %s\n", strerror(errno));
        exit(1);
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
    /* EPOLLIN means a new sock connection event */
    acceptChannel_.setReadCB(std::bind(&Acceptor::handleRead, this));
    acceptChannel_.enableRead();
}

Acceptor::~Acceptor() = default;

void Acceptor::handleRead() {
    assert(loop_->isInLoopThread());
    struct sockaddr_in peerAddr;
    socklen_t len = sizeof(peerAddr);
    int newFd = accept(sockFd_, reinterpret_cast<struct sockaddr *>(&peerAddr), &len);
    printf("%s->%s\n", __FILE__, __func__);
    if (newFd > 0) {
        setNonBlock(newFd);
        int opt = 1;
        ::setsockopt(newFd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR,
                     &opt, static_cast<socklen_t>(sizeof(opt)));
//        ::setsockopt(newFd, IPPROTO_TCP, TCP_NODELAY,
//                     &opt, static_cast<socklen_t>(sizeof(opt)));
        if (newConnectionCB_) {
            newConnectionCB_(newFd, &peerAddr);
        }
    } else {
        printf("accept() failed...\n");
    }
}


