/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpResponse.h"

HttpResponse::HttpResponse(bool close)
        : closeConnection_(close)
{ }

void HttpResponse:: toString(std::string &response) {
    response.append(version_ + " " + responseCode_ + " " + responseMsg_ + "\r\n");
    if (closeConnection_) {
        response.append("Connection: close\r\n");
    } else {
        response.append("Content-Length: " + std::to_string(getBodySize()) + "\r\n");
        response.append("Connection: Keep-Alive\r\n");
    }
    for (auto it = headers_.begin(); it !=  headers_.end(); ++it) {
        response.append(it->first);
        response.append(": ");
        response.append(it->second);
        response.append("\r\n");
    }
    response.append("\r\n");
    response.append(body_);
}