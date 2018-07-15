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
#include <sys/types.h>
#include <sys/socket.h>
#include "HttpContext.h"

static const int READ_BUFFER_SIZE = 10240;
static const int WRITE_BUFFER_SIZE = 40960;

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
    void setContext(std::shared_ptr<HttpContext> &context) {
        context_ = context;
    }

    EventLoop *getLoop() { return loop_; }
    const std::string getName() { return name_; }
    std::shared_ptr<HttpContext> getContext() const {
        return context_;
    }
    std::shared_ptr<HttpContext> getMutableContext() {
        return context_;
    }

    bool isConnected() { return state_ == CConnected; }

    void connectionDestroyed();
    void connectionEstablished();

    void send(const char *buf, int len);
    void stopRead();
    void startRead();
    void sendInLoop(const char *buf, int len);

    void shutdown();
    void shutdownInLoop();
private:
    enum State {CConnecting, CConnected, CDisconnecting, CDisconnected};

    void handleRead();
    void handleWrite();
    void handleClose(); /* realy closed by client */
    void handleError();

    void stopReadInLoop();
    void startReadInLoop();
    void setState(const State &s) { state_ = s; }

    EventLoop                   *loop_;
    int                         sockFd_;
    std::shared_ptr<Channel>    channel_;
    State                       state_;
    std::string                 name_;
    bool                        reading_;
    int                         readIdx_;
    int                         writeIdx_;
    std::shared_ptr<HttpContext>context_;
    struct stat                 fileState_;
    struct iovec                iv_[2];
    char                        readBuf_[READ_BUFFER_SIZE];
    char                        writeBuf_[WRITE_BUFFER_SIZE];

    const int                   ivCount_ = 2;

    CloseCB         closeCB_;
    MessageCB       messageCB_;
    ConnectionCB    connectionCB_;
    WriteCompleteCB writeCompleteCB_;
};

#endif //PROJECT_TCPCONN_H
