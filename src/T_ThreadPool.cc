/*
 * Created by by on 2018/7/12.
 *
 */

#include "EventLoop.h"
#include "EventLoopThreadPool.h"

void init() {
    printf("...init function\n");
}

void bus() {
    printf("^^^^^^^ busyness code ************\n");
}

void func_() {
    printf("$$$$$$$$$$$$$    @@@@@@@@@\n");
}

int main() {
    EventLoop loop;
    EventLoopThreadPool pool(&loop, "pool");
    pool.setNumThreads(3);
    pool.start();

    for (int i = 0; i < 3; ++i) {
       auto lo = pool.getNextLoop();
       lo->runInLoop(std::bind(bus));
    }
    sleep(2);
    for (int i = 0; i < 3; ++i) {
        auto lo = pool.getNextLoop();
        lo->runInLoop(std::bind(func_));
    }

    loop.loop();
}