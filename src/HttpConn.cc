//
// Created by Jemmy on 2018/6/11.
//

#include "HttpConn.h"
#include "CommonUtil.h"


const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file from this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The request file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

/* root path of web */
const char *docRoot = "/work";


int HttpConn::userCount = 0;
int HttpConn::epollFd = -1;

void HttpConn::closeConn(bool realClose) {
    if (realClose && (sockFd != -1)) {
//        removeFd(epollFd, sockFd);
        sockFd = -1;
        --userCount;
    }
}

void HttpConn::init(int sockFd, const sockaddr_in &addr) {
    this->sockFd = sockFd;
    this->address = addr;

    /* following two line just in order to avoid TIME_WAIT in debug */
    int reuse = 1;
    setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

//    addFd(epollFd, sockFd, true);
    ++userCount;

    init();
}

void HttpConn::init() {
    checkState = CHECK_STATE_REQUESTLINE;
    linger = false;

    method = GET;
    url = 0;
    version = 0;
    contentLength = 0;
    host = 0;
    startLine = 0;
    checkedIdx = 0;
    readIdx = 0;
    writeIdx = 0;
    memset(readBuf, '\0', READ_BUFFER_SIZE);
    memset(writeBuf, '\0', WRITE_BUFFER_SIZE);
    memset(readFile, '\0', FILENAME_LEN);
}

/* slave state machine */
/* check whether read a complete line */
HttpConn::LINE_STATUS HttpConn::parseLine() {
    char tmp;
    for (; checkedIdx < readIdx; ++checkedIdx) {
        tmp = readBuf[checkedIdx];
        if (tmp == '\r') {
            if ((checkedIdx + 1) == readIdx) {
                return LINE_OPEN;
            } else if (readBuf[checkedIdx+1] == '\n') {
                readBuf[checkedIdx++] = '\0';
                readBuf[checkedIdx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if (tmp == '\n') {
            if ((checkedIdx > 1) && (readBuf[checkedIdx-1] == '\r')) {
                readBuf[checkedIdx-1] = '\0';
                readBuf[checkedIdx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN; /* line is reading */
}

/* read continue */
bool HttpConn::read() {
    if (readIdx > READ_BUFFER_SIZE)
        return false;

    int bytesRead = 0;
    while (true) {
        bytesRead = recv(sockFd, readBuf + readIdx, READ_BUFFER_SIZE - readIdx, 0);
        if (bytesRead == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        } else if (bytesRead == 0) {
            return false;
        }
        readIdx += bytesRead;
    }
    return true;
}

/* parse http line, get method, url, version */
HttpConn::HTTP_CODE  HttpConn::parseRequestLine(char *text) {
    url = strpbrk(text, " \t");
    if (!url)
        return BAD_REQUEST;
    *url++ = '\0';

    char *md = text;
    if (strcasecmp(md, "GET") == 0)
        method = GET;
    else
        return BAD_REQUEST;

    url += strspn(url, " \t");
    version = strpbrk(url, " \t");
    if (!version)
        return BAD_REQUEST;
    *version++ = '\0';
    version += strspn(version, " \t");
    if (strcasecmp(version, "HTTP/1.1") != 0)
        return BAD_REQUEST;
    if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
    }
    if (!url || url[0] != '/')
        return BAD_REQUEST;

    checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::parseHeaders(char *text) {
    /* header has been parsed */
    if (text[0] == '\0') {
        if (contentLength != 0) {
            checkState = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST; /* get a complete http request */
    /* parse http Connection header */
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            linger = true;
        }
    /* parse http Content-Length header */
    } else if (strncasecmp(text, "Content-Length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        contentLength = atol(text);
    /* parse http Host header */
    } else if (strncasecmp(text, "Host:", 5) == 0) {
        text += 5;
        text += strspn(text, " \t");
        host = text;
    } else {
        printf("Oops! unknown header: %s\n", text);
    }

    return NO_REQUEST;
}

/* TODO we do not real parse content, just check whether it is read completely */
HttpConn::HTTP_CODE HttpConn::parseContent(char *text) {
    if (readIdx >= contentLength + checkedIdx) {
        text[contentLength] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

/* main state machine */
/* process client request line and doRequest */
HttpConn::HTTP_CODE HttpConn::processRead() {
    LINE_STATUS status = LINE_OK;
    HTTP_CODE  ret = NO_REQUEST;
    char *text = 0;

    while (((checkState == CHECK_STATE_CONTENT) && status == LINE_OK) ||
    (status = parseLine()) == LINE_OK){
        text = getLine();
        startLine = checkedIdx;
        printf("get 1 http line: %s\n", text);

        switch (checkState) {
            case CHECK_STATE_REQUESTLINE: {
                ret = parseRequestLine(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }
            case CHECK_STATE_HEADER: {
                ret = parseHeaders(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret == GET_REQUEST)
                    return doRequest();
                break;
            }
            case CHECK_STATE_CONTENT: {
                ret = parseContent(text);
                if (ret == GET_REQUEST)
                    return doRequest();
                status = LINE_OPEN;
                break;
            }
            default:
                return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

/* mmap the file which client request into memory */
HttpConn::HTTP_CODE HttpConn::doRequest() {
    strcpy(readFile, docRoot);
    int len = strlen(docRoot);
    strncpy(readFile + len, url, FILENAME_LEN - len - 1);
    if (stat(readFile, &fileStat) < 0)
        return NO_REQUEST;
    if (!(fileStat.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;
    if (S_ISDIR(fileStat.st_mode))
        return BAD_REQUEST;

    int fd = open(readFile, O_RDONLY);
    fileAddress = (char *)mmap(0, fileStat.st_size, PROT_READ,
            MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

void HttpConn::unmap() {
    if (fileAddress) {
        munmap(fileAddress, fileStat.st_size);
        fileAddress = 0;
    }
}

/* write the response to client */
// TODO readBuf, writeBuf clean up
bool HttpConn::write() {
    int tmp = 0;
    int bytes_have_send = 0;
    int bytes_to_send = writeIdx;
    if (bytes_to_send == 0) {
        modFd(epollFd, sockFd, EPOLLIN);
        init();
        return true;
    }
    while (true) {
        tmp = writev(sockFd, iv, ivCount);
        if (tmp <= -1) {
            /* if there is no space in TCP buffer, wait for next EPOLLOUT event,
             * even through server can't accept next request from the same fd immediately,
             * but we can make sure complete connection */
           if (errno == EAGAIN) {
               modFd(epollFd, sockFd, EPOLLOUT);
               return true;
           }
           unmap();
           return false;
        }
        bytes_to_send -= tmp;
        bytes_have_send += tmp;
        if (bytes_to_send <= bytes_have_send) {
            unmap();
            if (linger) {
                init();
                modFd(epollFd, sockFd, EPOLLIN);
                return true;
            }
        } else {
            modFd(epollFd, sockFd, EPOLLIN);
            return false;
        }
    }
}

bool HttpConn::addResponse(const char *format, ...) {
    if (writeIdx >= WRITE_BUFFER_SIZE)
        return false;
    va_list args;
    va_start(args, format);
    int len = vsnprintf(writeBuf + writeIdx, WRITE_BUFFER_SIZE - 1 - writeIdx,
            format, args);
    if (len >= WRITE_BUFFER_SIZE - 1 - writeIdx)
        return false;
    writeIdx += len;
    va_end(args);
    return true;
}

bool HttpConn::addStatusLine(int status, const char *title) {
    return addResponse("%s %d %s\r\n","HTTP/1.1", status, title);
}

bool HttpConn::addHeaders(int len) {
    addContentLength(len);
    addLinger();
    addBlankLine();
}

bool HttpConn::addContentLength(int len) {
    return addResponse("Content-Length: %d\r\n", len);
}

bool HttpConn::addLinger() {
    return addResponse("Connection: %s\r\n", (linger == true) ? "keep-alive" : "close");
}

bool HttpConn::addBlankLine() {
    return addResponse("%s", "\r\n");
}

bool HttpConn::addContent(const char *content) {
    return addResponse("%s", content);
}

bool HttpConn::processWrite(HttpConn::HTTP_CODE code) {
    switch (code) {
        case INTERNAL_ERROR: {
            addStatusLine(500, error_500_title);
            addHeaders(strlen(error_500_form));
            if (!addContent(error_500_form))
                return false;
            break;
        }
        case BAD_REQUEST: {
            addStatusLine(400, error_400_title);
            addHeaders(strlen(error_400_form));
            if (!addContent(error_400_form))
                return false;
            break;
        }
        case NO_REQUEST: {
            addStatusLine(404, error_404_title);
            addHeaders(strlen(error_404_form));
            if (!addContent(error_404_form))
                return false;
            break;
        }
        case FORBIDDEN_REQUEST: {
            addStatusLine(403, error_403_title);
            addHeaders(strlen(error_403_form));
            if (!addContent(error_403_form))
                return false;
            break;
        }
        case FILE_REQUEST: {
            addStatusLine(200, ok_200_title);
            if (fileStat.st_size != 0) {
                addHeaders(fileStat.st_size);
                iv[0].iov_base = writeBuf;
                iv[0].iov_len = writeIdx;
                iv[1].iov_base = fileAddress;
                iv[1].iov_len = fileStat.st_size;
                ivCount = 2;
                return true;
            } else {
                const char *okString = "<html><body>Welcome</body></html>";
                addHeaders(strlen(okString));
                if (!addContent(okString))
                    return false;
            }
        }
        default:
            return false;
    }
    iv[0].iov_base = writeBuf;
    iv[0].iov_len = writeIdx;
    ivCount = 1;
    return true;
}

/* invoke by threadPool thread, entry of process HTTP request */
void HttpConn::process() {
    HTTP_CODE readRet = processRead();
    if (readRet == NO_REQUEST) {
        modFd(epollFd, sockFd, EPOLLIN);
        return;
    }


    bool writeRet = processWrite(readRet);
    if (!writeRet) {
        closeConn();
    }
    modFd(epollFd, sockFd, EPOLLOUT);
}