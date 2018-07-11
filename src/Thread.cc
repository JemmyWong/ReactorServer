/*
 * Created by Jemmy on 2018/7/8.
 *
 */

#include "Thread.h"

struct ThreadData {
    typedef Thread::ThreadFunc ThreadFunc;

    ThreadFunc      func_;
    std::string     name_;
    pid_t           *tid_;
    CountDownLatch  *latch_;

    ThreadData(const ThreadFunc &func, std::string name,
               pid_t *tid, CountDownLatch *latch)
            :func_(func), name_(name), tid_(tid), latch_(latch) {}

    void runInThread() {
        *tid_ = gettid;
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;

        if (func_) func_(); /* ThreadInitCB */
    }
};

std::atomic<int> Thread::numCreated_;

Thread::Thread(const ThreadFunc &func, std::string name)
        : joined_(false),
          started_(false),
          tid_(0),
          pthread_(0),
          func_(func),
          name_(name),
          latch_(1)
{
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(pthread_);
    }
}

void* startFunc(void *obj) {
    ThreadData *data = static_cast<ThreadData *>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

void Thread::setDefaultName() {
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread-%d", numCreated_++);
        name_ = buf;
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    ThreadData *data = new ThreadData(func_, name_, &tid_, &latch_);
    if (pthread_create(&pthread_, NULL, &startFunc, data)) {
        started_ = false;
        delete data;
        printf("Failed in pthread_create\n");
    } else {
        latch_.wait();
        assert(tid_ > 0);
        printf("%s tid = %d created success\n", name_.c_str(), tid_);
    }
}

void Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;

    pthread_join(pthread_, NULL);
}

void func() {
    printf("Thread init function...\n");
}

/*
int main() {
    Thread thread1(std::bind(func));
    Thread thread2(std::bind(func));
    Thread thread3(std::bind(func));

    thread1.start();
    thread2.start();
    thread3.start();

    thread1.join();
    thread2.join();
    thread3.join();
}*/
