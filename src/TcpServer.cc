/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "TcpServer.h"

using namespace std::placeholders;

TcpServer::TcpServer(EventLoop *loop, const std::string name)
        : loop_(loop),
          ipPort_("9000"),
          name_(name),
          acceptor_(new Acceptor(loop_, "9000")),
          threadPool_(new EventLoopThreadPool(loop_, name)),
          mutex(),
          started_(false),
          nextConnId_(1)
{
    acceptor_->setNewConnectionCB(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
    assert(loop_->isInLoopThread());
    for (ConnectionMap::iterator it(connections_.begin());
            it != connections_.end(); ++it) {
        TcpConnectionPtr conn(it->second);
        it->second.reset();
        conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectionDestroyed, this, conn));
    }
}

void TcpServer::setThreadNum(int num) {
    assert(num >= 0);
    threadPool_->setNumThreads(num);
}

void TcpServer::start() {
    assert(!started_);
    started_ = true;
    assert(loop_->isInLoopThread());

    threadPool_->start(threadInitCB_);
}

void TcpServer::newConnection(int sockFd, void *addr) {
    struct sockaddr_in *peerAddr = static_cast<struct sockaddr_in *>(addr);

    assert(loop_->isInLoopThread());
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s*%d", "9000", nextConnId_++);
    std::string connName = name_ + buf;

    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockFd));
    conn->setMessageCb(messageCB_);
    conn->setConnectionCB(connectionCB_);
    conn->setWriteCompleteCB(writeCompleteCB_);
    /* thread safe guard by mutex */
    conn->setCloseCB(std::bind(&TcpServer::removeConnection, this, _1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    assert(loop_->isInLoopThread());
    {
        MutexLockGuard lock(mutex);
        assert(connections_.erase(conn->getName()) == 1);
    }
    conn->getLoop()->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, this));
}