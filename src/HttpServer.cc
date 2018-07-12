/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpServer.h"

HttpServer::HttpServer(EventLoop *loop, const std::string name, const std::string port)
        : server_(loop, name, port)
{ }

HttpServer::~HttpServer() = default;

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
    if (conn->CConnected) {
        std::shared_ptr<HttpContext> context(new HttpContext);
        conn->setContext(context);
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, char *buf, int len) {
    std::shared_ptr<HttpContext> context = conn->getMutableContext();
    if (context->parseRequest()) {
        const char *code = "HTTP/1.1 400 Bad Request\r\n\r\n";
        conn->send(code, (int)strlen(code));
        conn->shutDown();
    }
    if (context->getAll()) {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &) {

}

void HttpServer::start() {
    server_.start();
}

