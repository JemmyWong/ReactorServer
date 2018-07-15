/*
 * Created by Jemmy on 2018/7/14.
 *
 */

#include <EventLoop.h>
#include "HttpServer.h"

int main() {
    EventLoop loop;
    std::string name = "holy";
    HttpServer server(&loop, name);
    server.setThreadNum(3);
    server.start();

    loop.loop();
}