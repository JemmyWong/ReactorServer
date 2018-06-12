//
// Created by Jemmy on 2018/6/13.
//

#ifndef PROJECT_THREADPOOL_H
#define PROJECT_THREADPOOL_H

template<typename T>
class ThreadPool{
public:
    ThreadPool() {}
    ThreadPool(int size = 10) : threadSize(size) {

    }
private:
    int     threadSize;
    int     threadNumber;
    pthread_mutex_t mutex;
    ThreadJobQueue jobQueue;
};

#endif //PROJECT_THREADPOOL_H
