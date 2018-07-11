/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_TCPSERVER_H
#define PROJECT_TCPSERVER_H

#include "CommonUtil.h"
#include "Channel.h"
#include "TcpConnection.h"

class TcpServer {
public:

private:
    typedef std::map<std::string, TCPConnectionPtr> ConnectionMap;



    ConnectionMap  connections_;
};

#endif //PROJECT_TCPSERVER_H
