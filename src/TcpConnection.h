/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_TCPCONN_H
#define PROJECT_TCPCONN_H

#include "CommonUtil.h"
#include "Channel.h"
#include "EventLoop.h"
#include <sys/mman.h>   /* mmap, munmap */
#include <sys/stat.h>
#include <sys/uio.h>    /* struct iovec */

static const int READ_BUFFER_SIZE = 1024;
static const int WRITE_BUFFER_SIZE = 4096;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, const std::string &name, int fd);
    ~TcpConnection();

    void setCloseCB(const CloseCB &cb) {
        closeCB_ = cb;
    }
    void setMessageCb(const MessageCB &cb) {
        messageCB_ = cb;
    }
    void setConnectionCB(const ConnectionCB &cb) {
        connectionCB_ = cb;
    }
    void setWriteCompleteCB(const WriteCompleteCB &cb) {
        writeCompleteCB_ = cb;
    }
    void setContext(void *context) {
        context_ = context;
    }

    EventLoop *getLoop() { return loop_; }
    const std::string getName() { return name_; }
    void *getContext() const {
        return context_;
    }
    void *getMutableContext() {
        return context_;
    }

    void connectionDestroyed();
    void connectionEstablished();

    void send();
    void stopRead();
    void startRead();
    void sendInLoop();
private:
    enum State {CConnecting, CConnected, CDisconnecting, CDisconnected};

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void stopReadInLoop();
    void startReadInLoop();
    void setState(State &s) { state_ = s; }

    EventLoop                   *loop_;
    int                         sockFd_;
    std::shared_ptr<Channel>    channel_;
    State                       state_;
    std::string                 name_;
    bool                        reading_;
    int                         readIdx_;
    int                         writeIdx_;
    void                        *context_;
    char                        readBuf_[READ_BUFFER_SIZE];
    char                        writeBuf_[WRITE_BUFFER_SIZE];

    CloseCB         closeCB_;
    MessageCB       messageCB_;
    ConnectionCB    connectionCB_;
    WriteCompleteCB writeCompleteCB_;
};

#endif //PROJECT_TCPCONN_H
