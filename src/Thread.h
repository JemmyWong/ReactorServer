/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#ifndef PROJECT_THREAD_H
#define PROJECT_THREAD_H

#include <pthread.h>
#include <string>
#include <functional>
#include <atomic>
#include <cassert>
#include <sys/types.h>
#include "Mutex.h"
#include <unistd.h>
#include <sys/syscall.h>

#include "CommonUtil.h"

class Thread {
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(const ThreadFunc &, std::string name = std::string());
    Thread(Thread &) = delete;
    ~Thread();

    bool isStart() const { return started_; }
    bool isJoined() const { return joined_; }
    pid_t getTid() const { return tid_; }
    std::string &getName() {return name_; }

    void start();
    void join();

private:
    bool            joined_;
    bool            started_;
    pid_t           tid_;
    pthread_t       pthread_;
    ThreadFunc      func_;
    std::string     name_;
    CountDownLatch  latch_;

    void setDefaultName();

    static std::atomic<int> numCreated_;
};

#endif //PROJECT_THREAD_H
