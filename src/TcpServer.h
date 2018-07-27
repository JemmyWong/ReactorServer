/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_TCPSERVER_H
#define PROJECT_TCPSERVER_H

#include <atomic>
#include "Slog.h"
#include "Acceptor.h"
#include "CommonUtil.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

class TcpServer {
public:
    TcpServer(EventLoop *loop, std::string name = std::string(), std::string port = "9000");
    ~TcpServer();

    void setThreadNum(int num);
    void start();

    void setConnectionCB(const ConnectionCB &cb) {
        connectionCB_ = cb;
    }
    void setMessageCB(const MessageCB &cb) {
        messageCB_ = cb;
    }
    void setWriteCompleteCB(const WriteCompleteCB &cb) {
        writeCompleteCB_ = cb;
    }
    void setThreadInitCb(const ThreadInitCB &cb) {
        threadInitCB_ = cb;
    }

    EventLoop *getLoop() { return loop_; }
private:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    void newConnection(int sockFd, const void *peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    EventLoop                   *loop_;
    const std::string           ipPort_;
    const std::string           name_;
    std::shared_ptr<Acceptor>   acceptor_;
    ConnectionMap               connections_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    MessageCB               messageCB_;
    ConnectionCB            connectionCB_;
    WriteCompleteCB         writeCompleteCB_;
    ThreadInitCB            threadInitCB_;

    MutexLock               mutex;  /* guard connections_*/
    std::atomic<bool>       started_;
    int                     nextConnId_;

};

#endif //PROJECT_TCPSERVER_H
