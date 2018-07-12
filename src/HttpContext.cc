/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpContext.h"

char *HttpContext::getLine() {
    return buf_ + startLine_;
}

LineStatus HttpContext::parseLine() {
    char tmp;
    for (; checkedIdx_ < readIdx_; ++checkedIdx_) {
        tmp = buf_[checkedIdx_];
        if (tmp == '\r') {
            if ((checkedIdx_ + 1) == readIdx_) {
                return LINE_OPEN;
            } else if (buf_[checkedIdx_+1] == '\n') {
                buf_[checkedIdx_++] = '\0';
                buf_[checkedIdx_++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if (tmp == '\n') {
            if ((checkedIdx_ > 1) && (buf_[checkedIdx_-1] == '\r')) {
                buf_[checkedIdx_-1] = '\0';
                buf_[checkedIdx_++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN; /* line is reading */
}

bool HttpContext::parseRequest(const char *buf, int len) {
    LineStatus  state = LINE_OK;
    HttpCode    code = NO_REQUEST;
    char *text = 0;
    while (checkState_ == CHECK_STATE_CONTENT && state == LINE_OK
            || (state == parseLine()) == LINE_OK) {
        text = getLine();
        startLine_ = checkedIdx_;

        switch (checkState_) {
            case CHECK_STATE_REQUESTLINE:
                code = request_.parseRequestLine(text, checkedIdx_);
                break;
            case CHECK_STATE_HEADER:
                request_.parseHeader(text, checkedIdx_);
                break;
            case CHECK_STATE_CONTENT:
                request_.parseBody(text, checkedIdx_);
                break;
            default:
                return false;
        }
    }
    return true;
}