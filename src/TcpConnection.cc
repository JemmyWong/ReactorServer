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

/* the server actively sends data */
/* try to send data, if can not send whole data, save remaind data to conn's writeBuf_,
 * enable EPOLLOUT, send it next time */
void TcpConnection::send(const char *buf, int len) {
    if (state_ == CConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf, len);
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf, len));
        }
    }
}

void TcpConnection::sendInLoop(const char *buf, int len) {
    assert(loop_->isInLoopThread());
    int n = 0;
    int bytes_have_send = 0;
    int bytes_to_send = len;

    while (true) {
        n = ::send(channel_->getFd(), buf, len, 0);
        if (n <= -1) {
            /* send failed, send again */
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                channel_->enableWrite();
                getLoop()->runInLoop(
                        std::bind(&TcpConnection::sendInLoop,this, buf + bytes_have_send, len));
                return;
            }
            return;
        }
        bytes_to_send -= n;
        bytes_have_send += n;
        /* data send have completed */
        if (bytes_to_send <= bytes_have_send) {
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

void TcpConnection::connectionDestroyed() {
    assert(loop_->isInLoopThread());
    if (state_ == CConnected) {
        setState(CDisconnected);
        channel_->disableAll();
        connectionCB_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead() {
    assert(loop_->isInLoopThread());
    int byteRead = 0;

    /* at EPOLLET model must read all data at once */
    while (true) {
        byteRead = read(channel_->getFd(), readBuf_ + readIdx_, READ_BUFFER_SIZE - readIdx_);
        if (byteRead == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                handleError();
                return;
            }
        } else if (byteRead == 0) { /* peer closed */
            return;
        }
        readIdx_ += byteRead;
        if (readIdx_ > READ_BUFFER_SIZE) {
            printf("WARNING: TcpConnection fd[%d] readBuf_ overflow\n", channel_->getFd());
            return;
        }
    }
    messageCB_(shared_from_this(), readBuf_, readIdx_);
}

void TcpConnection::handleWrite() {
    assert(loop_->isInLoopThread());
    if (channel_->isWriting()) {
        int n = 0;
        int bytes_have_send = 0;
        int bytes_to_send = writeIdx_;

        bool ret = false;
        while (true) {
            n = writev(channel_->getFd(), iv_, ivCount_);
            if (n <= -1) {
                /* if there is no space in TCP buffer, wait for next EPOLLOUT event,
                 * even through server can't accept next request from the same fd immediately,
                 * but we can make sure complete connection */
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    channel_->enableWrite();
                }
//                unmap();
                return;
            }
            bytes_to_send -= n;
            bytes_have_send += n;
            /* data send completed */
            if (bytes_to_send <= bytes_have_send) {
//                unmap();
                break;
            }
        }
        writeCompleteCB_(shared_from_this());
    }
}

void TcpConnection::handleClose() {
    assert(loop_->isInLoopThread());
    assert(state_ == CConnected || state_ == CConnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(CDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCB_(guardThis);
    closeCB_(guardThis);
}

void TcpConnection::handleError() {
    printf("TcpConnection Fd[%d] name[%s] Error: %s\n", sockFd_, name_.c_str(), strerror(errno));
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