/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#ifndef PROJECT_HTTPREQUEST_H
#define PROJECT_HTTPREQUEST_H

#include "CommonUtil.h"
#include <map>
#include <string>

class TcpServer;
class HttpCOntext;
class HttpRequest;
class HttpResponse;

class HttpRequest {
public:
    HttpRequest();

    void setVersion(const char *begin, const char *end) {
        version_ = std::string(begin, end);
    }
    bool setMethod(const char *begin, const char *end) {
        method_ = std::string(begin, end);
        return true;
    }

    HttpCode parseBody(char *text, int len);
    HttpCode parseHeader(char *text, int len);
    HttpCode parseRequestLine(char *text, int len);

private:
    std::string         method_;
    std::string         path_;
    std::string         query_;
    std::string         version_;
    std::map<std::string, std::string> headers_;
};

#endif //PROJECT_HTTPREQUEST_H
