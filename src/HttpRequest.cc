/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpRequest.h"

HttpRequest::HttpRequest() : method_(GET_REQUEST), version_(HTTP10){ };

HttpCode HttpRequest::parseRequestLine(char *text, int len) {

}

HttpCode HttpRequest::parseHeader(char *text, int len) {

}

HttpCode HttpRequest::parseBody(char *text, int len) {

}