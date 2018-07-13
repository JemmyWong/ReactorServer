/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#ifndef PROJECT_HTTPCONTEXT_H
#define PROJECT_HTTPCONTEXT_H

#include "HttpRequest.h"
#include <algorithm>
#include <string>
#include <string.h>

class HttpContext {
public:
    HttpContext();

    void reset();

    /* use state machine to parse requestLine, head and content */
    HttpCode parseRequest(const char *buf, int len);
    HttpCode parseBody(char *text, int len);
    HttpCode parseHeader(char *text, int len);
    HttpCode parseRequestLine(char *text, int len);

    const HttpRequest getRequest() const { return request_; }
private:
    char * getLine();
    LineStatus parseLine();

    char        *buf_;
    int         readIdx_;
    int         checkedIdx_;
    int         startLine_;
    HttpCode    httpCode_;
    CheckState  checkState_;
    HttpRequest request_;
};

#endif //PROJECT_HTTPCONTEXT_H
