/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpRequest.h"

HttpRequest::HttpRequest()
        : requestCode_(NO_REQUEST),
          method_(),
          path_(),
          query_(),
          version_(),
          headers_()
{ }

bool HttpRequest::addHeader(const char *begin, const char *colon, const char *end) {
    std::string key(begin, (size_t)(colon - begin));
    ++colon;
    while (begin < end && isspace(*colon)) ++ colon;

    std::string value(colon, (size_t )(end - colon));
    while (!value.empty() && isspace(value[value.size()-1]))
        value.reserve(value.size()-1);
    headers_[key] = value;
    return true;
}

void HttpRequest::reset() {
    requestCode_ = NO_REQUEST;
    method_.clear();
    path_.clear();
    query_.clear();
    version_.clear();
    headers_.clear();
}