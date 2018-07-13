/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpRequest.h"

HttpRequest::HttpRequest() : method_(GET_REQUEST), version_(HTTP10){ };

bool HttpRequest::addHeader(const char *begin, const char *colon, const char *end) {
    std::string key(begin, colon);
    ++colon;
    while (begin < end && isspace(*colon)) ++ colon;

    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size()-1]))
        value.reserve(value.size()-1);
    headers_[key] = value;
    return true;
}

void HttpRequest::reset() {
    method_.clear();
    path_.clear();
    query_.clear();
    version_.clear();
    headers_.clear();
}