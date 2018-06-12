//
// Created by Jemmy on 2018/6/11.
//

#ifndef PROJECT_HTTPCONN_H
#define PROJECT_HTTPCONN_H

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/mman.h>   /* mmap, munmap */
#include <sys/stat.h>
#include <sys/uio.h>    /* struct iovec */
#include <cstring>
#include <cstdlib>      /* atol */
#include <cerrno>
#include <stdio.h>
#include <cstdarg>      /* va_list */
#include <fcntl.h>

class HttpConn {
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;

    /* http request method, only support GET */
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE,
            TRACE, OPTIONS, CONNECT, PATCH};

    /* state of the state machine */
    enum CHECK_STATE {CHECK_STATE_REQUESTLINE = 0,
            CHECK_STATE_HEADER, CHECK_STATE_CONTENT};

    /* result of http request processed */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST,
            NO_RESOURCE, FORBIDDEN_REQUEST,_REQUEST, FILE_REQUEST,
            INTERNAL_ERROR, CLOSED_CONNECTION};

    /* state of line read */
    enum LINE_STATUS {
            LINE_OK = 0,    /* complete request line */
            LINE_BAD,       /* incomplete request line */
            LINE_OPEN       /* line is reading */
    };

public:
    HttpConn() {};
    ~HttpConn() {};
public:
    /* init a connection */
    void init(int sockFd, const sockaddr_in &addr);

    void closeConn(bool realClose = true);

    /* process user request */
    void process();
    /* non-blocking */
    bool read();
    bool write();

private:
    void init();
    /* parse request */
    HTTP_CODE processRead();    /* 1. main state machine */
    /* response http request */
    bool processWrite(HTTP_CODE code);

    /* called by processRead to parse http request */
    LINE_STATUS parseLine();    /* 2. slave state machine, check whether read a complete line */
    HTTP_CODE  parseRequestLine(char *text);
    HTTP_CODE  parseHeaders(char *text);
    HTTP_CODE  parseContent(char *text);
    HTTP_CODE  doRequest();
    char *getLine() {return readBuf + startLine; };

    /* called by processWrite to make a response */
    void unmap();
    bool addResponse(const char *format, ...);
    bool addStatusLine(int status, const char *title);
    bool addHeaders(int len);
    bool addContent(const char *content);
    bool addContentLength(int len);
    bool addLinger();
    bool addBlankLine();

public:
    static int epollFd;
    static int userCount;
private:
    int         sockFd;
    sockaddr_in address;

    char        readBuf[READ_BUFFER_SIZE];
    int         readIdx;    /* position of the char has read in the buf*/
    int         startLine;  /* position of line parsing */
    int         checkedIdx; /* position of the char has checked in the buf */
    CHECK_STATE checkState; /* status of status machine */

    char        writeBuf[WRITE_BUFFER_SIZE];
    int         writeIdx;   /* num of bytes need send */

    METHOD      method;         /* GET, POST */
    char        *url;           /* file name requested */
    char        *version;       /* HTTP/1.1 */
    char        *host;
    int         contentLength;  /* length of request message */
    bool        linger;         /* keep connection or no*/

    char        readFile[FILENAME_LEN]; /* full path of file requested by user */
    char        *fileAddress;   /* position of file requested and mmap in the memory */
    struct stat fileStat;
    struct iovec iv[2];
    int         ivCount;
};


#endif //PROJECT_HTTPCONN_H
