//
// Created by Jemmy on 2018/6/11.
//

#ifndef PROJECT_HTTPCONN_H
#define PROJECT_HTTPCONN_H

#include <sys/types.h>
#include <netietn/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/mman.h>
#include <cerrno>
#include <stdarg.h>

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
            NO_RESOURCE, FORBIDDEN,_REQUEST, FILE_REQUEST,
            INTERNAL_ERROR, CLOSED_CONNECTION};

    /* state of line read */
    enum LINE_STATUS {LINE_OK = 0, LINE_BND, LINE_OPEN};

public:
    HttpConn();
    ~HttpConn();
public:
    /* init a connection */
    void init(int sockFd, const sockaddr_int &addr);

    void closeConn(bool realClose = true);

    /* process user request */
    void process();
    /* non-blocking */
    bool read();
    bool write();

private:
    void init();
    /* parse request */
    HTTP_CODE processRead();
    /* response http */
    bool processWrite(HTTP_CODE code);

    /* called by processRead to parse http request */
    HTTP_CODE  parseRequestLine(char *text);
    HTTP_CODE  parseHeaders(char *text);
    HTTP_CODE  parseContent(char *text);
    HTTP_CODE doRequest();
    char *getLine() {return readBuf + startLine; };
    LINE_STATUS parseLine();

    /* called by processWrite to make a response */
     void unmap();
     bool addResponse(const char *format, ...);
     bool addContent(const char *content);
     bool addStatusLine(int status, const char *title);
     bool adContentLength(int len);
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
    int         checkedIdx; /* position of the char in the buf */
    int         startLine;  /* position of line parsing */

    char        writeBuf[WRITE_BUFFER_SIZE];
    int         writeIdx;   /* num of bytes need send */

    CHECK_STATE checkState; /* status of status machine */
    METHOD      method;     /* GET, POST */

    char        readFile[FILENAME_LEN]; /* full path of file requested by user */
    char        *url;           /* file name requested */
    char        *version;       /* HTTP/1.1 */
    char        *host;
    int         contentLength;  /* length of request message */
    bool        linger;         /* keep connection or no*/

    char        *fileAddress;   /* position of file requested and mmap in the memory */
    struct stat fileStat;
    struct iovec iv[2];
    int         ivCount;
};


#endif //PROJECT_HTTPCONN_H
