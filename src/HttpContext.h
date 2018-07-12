/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#ifndef PROJECT_HTTPCONTEXT_H
#define PROJECT_HTTPCONTEXT_H

#include "HttpRequest.h"

class HttpContext {
public:
    /* use state machine to parse requestLine, head and content */
    bool parseRequest(const char *buf, int len);
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
