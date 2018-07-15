/*
 * Created by Jemmy on 2018/7/11.
 *
 */

#include "HttpResponse.h"

HttpResponse::HttpResponse(bool close)
        : closeConnection_(close)
{ }

const std::string HttpResponse:: toStting() {
    std::string response;
    response.append(version_ + " " + responseCode_ + " " + responseMsg_ + "\r\n");
//    for (auto it = headers_.begin(); it !=  headers_.end(); ++it) {
//        response.append(it->first);
//        response.append(": ");
//        response.append(it->second);
//        response.append("\r\n");
//    }
    response.append("\r\n");
    return response;
}