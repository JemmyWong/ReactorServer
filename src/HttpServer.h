/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#ifndef PROJECT_HTTPSERVER_H
#define PROJECT_HTTPSERVER_H

#include <fstream>
#include <sstream>
#include <iostream>

#include "Slog.h"
#include "CommonUtil.h"
#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

using namespace std::placeholders;

class HttpServer {
public:
    HttpServer(EventLoop *loop, std::string name = std::string(), std::string port = "9000");
    ~HttpServer();

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, const char *buf, int len);
    void onRequest(const TcpConnectionPtr &conn, const HttpRequest &);

    EventLoop *getLoop() { return server_.getLoop(); }

    void setHttpCB(const HttpCB &cb) { httpCB_ = cb; }
    void setThreadNum(int num) {
        server_.setThreadNum(num);
    }

    void start();

private:
    void processError(const HttpRequest &req, HttpResponse *response);
    void defaultHttpCB(const HttpRequest &req, HttpResponse *response);

    TcpServer   server_;
    HttpCB      httpCB_; /* user function to process client request */
};

#endif //PROJECT_HTTPSERVER_H
