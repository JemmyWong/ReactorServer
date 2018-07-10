/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "TcpConnection.h"

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int fd)
        :  loop_(loop),
           sockFd_(fd),
           channel_(new Channel(loop_, fd)),
           state_(CConnecting),
           name_(name),
           reading_(true),
           readIdx_(0),
           writeIdx_(0)
{
    channel_->setReadCB(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCB(std::bind(&TcpConnection::handleWrite, this));
    channel_->setErrorCB(std::bind(&TcpConnection::handleError, this));
    channel_->setCloseCB(std::bind(&TcpConnection::handleClose, this));
}

TcpConnection::~TcpConnection() {
    assert(state_ == CDisconnected);
}

void TcpConnection::send() {
    if (state_ == CConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop();
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this));
        }
    }
}

// TODO
void TcpConnection::sendInLoop() {
    assert(loop_->isInLoopThread());
    int tmp = 0;
    int bytes_have_send = 0;
    int bytes_to_send = writeIdx_;
    if (bytes_to_send == 0) {
//        modFd(epollFd, sockFd, EPOLLIN);
        channel_->enableRead();
        return;
    }
    while (true) {
        tmp = writev(sockFd, iv, ivCount);
        if (tmp <= -1) {
            /* if there is no space in TCP buffer, wait for next EPOLLOUT event,
             * even through server can't accept next request from the same fd immediately,
             * but we can make sure complete connection */
            if (errno == EAGAIN) {
//                modFd(epollFd, sockFd, EPOLLOUT);
                channel_->enableWrite();
                return;
            }
            unmap();
            return;
        }
        bytes_to_send -= tmp;
        bytes_have_send += tmp;
        if (bytes_to_send <= bytes_have_send) {
            unmap();
            if (linger) {
                init();
                modFd(epollFd, sockFd, EPOLLIN);
                return;
            }
        } else {
            modFd(epollFd, sockFd, EPOLLIN);
            return;
        }
    }
}

void TcpConnection::connectionEstablished() {
    assert(loop_->isInLoopThread());
    assert(state_ == CConnecting);
    setState(CConnected);
    channel_->tie(shared_from_this());
    channel_->enableRead();
    connectionCB_(shared_from_this());
}

void TcpConnection::handleRead() {
    assert(loop_->isInLoopThread());
    int byteRead = 0;
    while (true) {
        byteRead = read(channel_->getFd(), readBuf_ + readIdx_, READ_BUFFER_SIZE - readIdx_);
        if (byteRead == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
        } else if (byteRead == 0) {
            return;
        }
        readIdx_ += byteRead;
        if (readIdx_ > READ_BUFFER_SIZE)
            return;
    }
}

void TcpConnection::handleWrite() {

}

void TcpConnection::handleClose() {

}

void TcpConnection::handleError() {

}

void TcpConnection::startRead() {
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
    assert(loop_->isInLoopThread());
    if (!reading_ && !channel_->isReading()) {
        channel_->enableRead();
        reading_ = true;
    }
}

void TcpConnection::stopRead() {
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
    assert(loop_->isInLoopThread());
    if (reading_ || channel_->isReading()) {
        channel_->disableRead();
        reading_ = false;
    }
}