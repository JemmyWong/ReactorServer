/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#ifndef PROJECT_HTTPSERVER_H
#define PROJECT_HTTPSERVER_H

#include "CommonUtil.h"
#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpServer {
public:
    HttpServer(EventLoop *loop, const std::string name = std::string(), const std::string port = "9000");
    ~HttpServer();

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, char *buf, int len);
    void onRequest(const TcpConnectionPtr &conn, const HttpRequest &);

    EventLoop *getLoop() { return server_.getLoop(); }

    void setHttpCB(const HttpCB &cb) { httpCB_ = cb; }

    void start();
private:
    TcpServer   server_;
    HttpCB      httpCB_; /* user function to process client request */
};

#endif //PROJECT_HTTPSERVER_H
