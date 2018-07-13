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
    if (context->parseRequest(buf, len) != GET_REQUEST) {
        const char *code = "HTTP/1.1 400 Bad Request\r\n\r\n";
        conn->send(code, (int)strlen(code));
//        conn->shutDown();
    } else {
        onRequest(conn, context->getRequest());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req) {
    const std::string connection = req.getHeader("Connection");
    bool close = connection == "close"
                               ||(req.getVersion() == "HTTP/1.0" && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCB_(req, &response);

    const char *data = response.toStting().c_str();
    conn->send(data, (int)strlen(data));
//    if (response.isCloseConnection()) {
//        conn->shutDown();
//    }
}

void HttpServer::defaultHttpCB(const HttpRequest &req, HttpResponse *response) {
    HttpCode requestCode = req.getRequestCode();
    if (requestCode != GET_REQUEST) {
        processError(req, response);
    } else {

    }
}

void HttpServer::processError(const HttpRequest &req, HttpResponse *response) {
    HttpCode code = req.getRequestCode();
    response->setVersion(req.getVersion());
    response->addHeaders(req.getHeaders());

    switch (code) {
        case NO_REQUEST:
            response->setResponseCode(error_404_title);
            response->setResposneMsg(error_404_form);
            break;
        case BAD_REQUEST:
            response->setResponseCode(error_404_title);
            response->setResposneMsg(error_404_form);
            break;
        case INTERNAL_ERROR:
            response->setResponseCode(error_404_title);
            response->setResposneMsg(error_404_form);
            break;
        case NO_RESOURCE:
            response->setResponseCode(error_404_title);
            response->setResposneMsg(error_404_form);
            break;
        default:
            response->setResponseCode(error_404_title);
            response->setResposneMsg(error_404_form);
            break;
    }
}

void HttpServer::start() {
    server_.start();
}

