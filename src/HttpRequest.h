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

    bool setMethod(const char *begin, const char *end) {
        method_ = std::string(begin, end);
        return !method_.empty();
    }
    bool setPath(const char *begin, const char *end) {
        path_ = std::string(begin, end);
        return !path_.empty();
    }
    bool setVersion(const char *begin, const char *end) {
        version_ = std::string(begin, end);
        return version_.empty();
    }
    bool setQuery(const char *begin , const char *end) {
        query_ = std::string(begin, end);
        return !query_.empty();
    }
    bool setRequestCode (HttpCode code) {
        requestCode_ = code;
        return true;
    }
    bool addHeader(const char *begin, const char *colon, const char *end);

    void reset();

    HttpCode getRequestCode()   const { return requestCode_; };
    std::string &getPath()      const { return path_; }
    std::string &getQuery()     const { return query_; }
    std::string &getMethod()    const { return method_; }
    std::string &getVersion()   const { return version_; }
    const std::map<std::string, std::string> &getHeaders() const { return headers_; };
    const std::string &getHeader(const std::string &key) const { return headers_[key]; }

private:
    HttpCode            requestCode_;
    std::string         method_;
    std::string         path_;
    std::string         query_;
    std::string         version_;
    std::map<std::string, std::string> headers_;
};

#endif //PROJECT_HTTPREQUEST_H
