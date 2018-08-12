/*
 * Created by Jemmy on 2018/7/26.
 *
 */

#include "Slog.h"
#include "EventLoop.h"
#include "HttpServer.h"
#include "ConfigUtil.h"

using namespace std;

extern ConfigFile config;

int main() {
    config.init("config.conf");
    int ret = slog_init();
    if (ret) {
        fprintf(stderr, "log_init error: %s\n", strerror(errno));
        exit(0);
    }
    EventLoop loop;
    std::string name = "holy";
    HttpServer server(&loop, name);
    server.setThreadNum(3);
    server.start();

    loop.loop();
}

