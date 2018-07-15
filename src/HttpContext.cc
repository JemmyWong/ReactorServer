/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpContext.h"

HttpContext::HttpContext()
        : checkedIdx_(0),
          startLine_(0),
          httpCode_(NO_REQUEST),
          checkState_(CHECK_STATE_REQUESTLINE),
          request_()
{ }

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

HttpCode HttpContext::parseRequest(const char *buf, int len) {
    printf("%s->%s\n", __FILE__, __func__);
    LineStatus  state = LINE_OK;
    HttpCode    code = NO_REQUEST;
    char *text = nullptr;
    int le = 0;
    while (((checkState_ == CHECK_STATE_CONTENT) && state == LINE_OK)
            || (state = parseLine()) == LINE_OK) {
        text = getLine();
        le = checkedIdx_ - startLine_ - 2;
        startLine_ = checkedIdx_;
//        printf("get one request lien: %s\n", text);

        switch (checkState_) {
            case CHECK_STATE_REQUESTLINE:
                code = parseRequestLine(text, le);
                if (code == BAD_REQUEST) {
                    request_.setRequestCode(BAD_REQUEST);
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_HEADER:
                code = parseHeader(text, le);
                if (code == BAD_REQUEST) {
                    request_.setRequestCode(BAD_REQUEST);
                    return BAD_REQUEST;
                } /*else if (code == GET_REQUEST) {
                    request_.setRequestCode(GET_REQUEST);
                    return GET_REQUEST;
                }*/
                break;
            case CHECK_STATE_CONTENT:
                code = parseBody(text, le);
                if (code == BAD_REQUEST) {
                    request_.setRequestCode(BAD_REQUEST);
                    return BAD_REQUEST;
                } else if (code == GET_REQUEST) {
                    request_.setRequestCode(GET_REQUEST);
                    return GET_REQUEST;
                }
                break;
            default:
                request_.setRequestCode(INTERNAL_ERROR);
                return INTERNAL_ERROR;
        }
    }
    request_.setRequestCode(GET_REQUEST);
    return GET_REQUEST;
}

HttpCode HttpContext::parseRequestLine(char *text, int len) {
    const char *begin = text;
    const char *end = text + len;
    const char *space = std::find(begin, end, ' ');
    if (space != end && request_.setMethod(begin, space)) {
        begin = space + 1;
        space = std::find(begin, end, ' ');
        if (space != end) {
            const char *question = std::find(begin, space, '?');
            if (question != space) {
                request_.setPath(begin, question);
                request_.setQuery(question, space);
            } else {
                request_.setPath(begin, space);
            }
        } else {
            return BAD_REQUEST;
        }
        begin = space + 1;
        bool ok = end - begin == 8 && std::equal(begin, end - 1, "HTTP/1.");
        if (ok) {
                request_.setVersion(begin, end);
        } else {
            return BAD_REQUEST;
        }
    } else {
        return BAD_REQUEST;
    }
    checkState_ = CHECK_STATE_HEADER;
    return GET_REQUEST;
}

HttpCode HttpContext::parseHeader(char *text, int len) {
    const char *begin = text;
    const char *end = text + len;
    const char *colon = std::find(begin, end, ':');
    if (colon != end && request_.addHeader(begin, colon, end)) {
        return GET_REQUEST;
    } else if (colon == end) {
        checkState_ = CHECK_STATE_CONTENT;
        return GET_REQUEST;
    }
    return BAD_REQUEST;
}

/* TODO parseBody */
HttpCode HttpContext::parseBody(char *text, int len) {
//    if (len == 0) {
//        return NO_REQUEST;
//    }
    return GET_REQUEST;
}

void HttpContext::reset() {
    buf_ = nullptr;
    readIdx_ = 0;
    checkedIdx_ = 0;
    startLine_ = 0;
    httpCode_ = NO_REQUEST;
    checkState_ = CHECK_STATE_REQUESTLINE;
    request_.reset();
}