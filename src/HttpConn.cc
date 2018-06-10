//
// Created by Jemmy on 2018/6/11.
//

#include "HttpConn.h"
#include "commonUtil.h


const char *ok_200_titile = "OK";
const char *error_404_titile = "Bad Request";
const char *error_404_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file from this server.\n";
const char *errro_404_title = "Not Found";
const char *errror_404_form = "The request file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

/* root path of web */
const char *docRoot = "/var/www/html/";


int HttpConn::userCount = 0;
int HttpConn::epollFd = -1;

void HttpConn::closeConn(bool realClose) {
    if (realClose && (sockFd != -1)) {
        removeFd(epollFd, sockFd);
        sockFd = -1;
        --userCount;
    }
}

