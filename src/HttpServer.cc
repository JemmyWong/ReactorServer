/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpServer.h"

HttpServer::HttpServer(EventLoop *loop, const std::string name, const std::string port)
        : server_(loop, name, port),
          httpCB_(std::bind(&HttpServer::defaultHttpCB, this, _1, _2))
{
    server_.setConnectionCB(std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCB(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer() = default;

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
    assert(conn->isConnected());
    std::shared_ptr<HttpContext> context(new HttpContext);
    conn->setContext(context);
    printf("%s->%s\n", __FILE__, __func__);
    slog_info("trace...");
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, const char *buf, int len) {
    printf("%s -> %s\n", __FILE__, __func__);
    slog_info("trace...");
    std::shared_ptr<HttpContext> context = conn->getMutableContext();
//    if (context->parseRequest(buf, len) != GET_REQUEST) {
//        const char *code = "HTTP/1.1 400 Bad Request\r\n\r\n";
//        conn->send(code, (int)strlen(code));
//        conn->shutDown();
//    } else {
//        onRequest(conn, context->getRequest());
//        context->reset();
//    }
    context->parseRequest(buf, len);
    onRequest(conn, context->getRequest());
    context->reset();
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req) {
    printf("%s->%s\n", __FILE__, __func__);
    slog_info("trace...");
    std::string connection = req.getHeader("Connection");
    bool close = connection == "close"
                               ||(req.getVersion() == "HTTP/1.0"
                                                      && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCB_(req, &response);

    std::string data = response.toStting();
    data.append(response.getBody());
    conn->send(data.c_str(), (int)data.size());
    if (response.isCloseConnection()) {
        conn->shutdown();
    }
}

void HttpServer::defaultHttpCB(const HttpRequest &req, HttpResponse *response) {
    printf("%s->%s\n", __FILE__, __func__);
    slog_info("trace...");
    HttpCode requestCode = req.getRequestCode();
    if (requestCode != GET_REQUEST) {
        processError(req, response);
    } else {
        response->setVersion(req.getVersion());
        response->addHeader("Server", "47.96.106.37:9000");
        response->addHeader("Content-Type", "text/html charset=utf-8");

        struct stat fileStat;
        std::string filePath = "/root" + req.getPath();
        if (stat(filePath.c_str(), &fileStat) < 0) {
            response->setResponseCode("400");
            response->setResposneMsg(HTTP::error_400_title);
            response->setBody(HTTP::error_400_form);
            response->addHeader("Content-Length", std::to_string(response->getBodySize()));
            return;
        }
        if (!(fileStat.st_mode & S_IROTH)){
            response->setResponseCode("403");
            response->setResposneMsg(HTTP::error_403_title);
            response->setBody(HTTP::error_403_form);
            response->addHeader("Content-Length", std::to_string(response->getBodySize()));
            return;
        }
        if (S_ISDIR(fileStat.st_mode)) {
            response->setResponseCode("400");
            response->setResposneMsg(HTTP::error_400_title);
            response->setBody(HTTP::error_400_form);
            response->addHeader("Content-Length", std::to_string(response->getBodySize()));
            return;
        }

//        int fd = open(filePath.c_str(), O_RDONLY);
//        char *fileAddress = (char *)mmap(0, fileStat.st_size, PROT_READ,
//                                   MAP_PRIVATE, fd, 0);
//        close(fd);
        std::ifstream file(filePath);
        std::stringstream buf;
        buf << file.rdbuf();
        std::string content(buf.str());
        response->setResponseCode("200");
        response->setResposneMsg(HTTP::ok_200_title);
        response->setBody(std::move(content));
        response->addHeader("Content-Length", std::to_string(response->getBodySize()));
    }
}

void HttpServer::processError(const HttpRequest &req, HttpResponse *response) {
    slog_info("trace...");
    printf("%s->%s\n", __FILE__, __func__);
    HttpCode code = req.getRequestCode();
    response->setVersion(req.getVersion());
    response->addHeader("Server", "47.96.106.37:9000");
    response->addHeader("Content-Type", "text/html charset=utf-8");

    switch (code) {
        case NO_REQUEST:
            response->setResponseCode("400");
            response->setResposneMsg(HTTP::error_404_title);
            response->setBody(HTTP::error_400_form);
            break;
        case BAD_REQUEST:
            response->setResponseCode("404");
            response->setResposneMsg(HTTP::error_404_title);
            response->setBody(HTTP::error_404_form);
            break;
        case INTERNAL_ERROR:
            response->setResponseCode("500");
            response->setResposneMsg(HTTP::error_500_title);
            response->setBody(HTTP::error_500_form);
            break;
        case NO_RESOURCE:
            response->setResponseCode("403");
            response->setResposneMsg(HTTP::error_403_title);
            response->setBody(HTTP::error_403_form);
            break;
        default:
            response->setResponseCode("404");
            response->setResposneMsg(HTTP::error_404_title);
            response->setBody(HTTP::error_404_form);
            break;
    }
    response->addHeader("Content-Length", std::to_string(response->getBodySize()));
}

void HttpServer::start() {
    server_.start();
}

