#ifndef _TIMER_H_
#define _TIMER_H_

#include <iostream>
#include <netinet/in.h>
#include <time.h>
#include <cassert>
#include <pthread.h>

#include "../include/slog.h"
#include "../include/eventhandler.h"

using std::exception;

#define BUFFER_SIZE 64

class Timer;

void handle_timer_event(handle_event_msg_t *handle_event_msg);

event_handler_t *create_timer_handler(reactor_t *reactor);

void addTimer(int delay);

struct ClientData {
    int         fd;
    Timer       *timer;
    sockaddr_in address;
    char buf[BUFFER_SIZE];
};

class Timer {
public:
    Timer(int delay) {
        args = nullptr;
        cb_func = nullptr;
        expire = time(nullptr) + delay;
    }

public:
    void *args;
//    time_t expire;
    int expire;
    void (*cb_func)(void *);
};

class TimerHeap {
public:
    TimerHeap(int cap) throw(std::exception): capacity(cap), cur_size(0) {
        array = new Timer *[capacity];
        if (!array) {
            throw std::exception();
        }
        for (int i = 0; i < capacity; ++i) {
            array[i] = NULL;
        }
    }
//  initialized with an existing array
    TimerHeap(Timer **init_array, int size, int cap) throw(std::exception)
    : cur_size(size), capacity(cap) {
        if (capacity < size) {
            throw std::exception();
        }
        array = new Timer *[capacity];
        if (!array) throw std::exception();
        if (size != 0) {
            for (int i = 0; i < size; ++i) {
                array[i] = init_array[i];
            }
//          adjust heap
            for (int i = (cur_size-1)/2; i >=0; --i) {
                percolate_down(i);
            }
        }
    }

    ~TimerHeap() {
        for (int i = 0; i < cur_size; ++i) {
            delete array[i];
        }
        delete [] array;
    }

public:
    void add_timer(Timer *timer) throw(std::exception) {
        if (!timer) return;

        pthread_mutex_lock(&mutex);
        if (cur_size >= capacity)   resize();

        int hole = cur_size++;
        int parent = 0;
        for (; hole > 0; hole = parent) {
            parent = (hole - 1) / 2;
            if (array[parent]->expire <= timer->expire)
                break;
            array[hole] = array[parent];
        }
        array[hole] = timer;
        pthread_mutex_unlock(&mutex);

        printf("new timer added, expire<%d>, heap capacity<%d>\n", timer->expire, capacity);
        slog_info("new timer added, expire<%d>, heap capacity<%d>", timer->expire, capacity);
    }

//  delay destroy
    void del_timer(Timer *timer) {
        if (!timer) return;
//        timer->cb_func = NULL; TODO del_timer

        pthread_mutex_lock(&mutex);
        for (int i = (cur_size-1)/2; i >=0; --i) {
            percolate_down(i);
        }
        pthread_mutex_unlock(&mutex);
    }

    Timer *top() const {
        if (empty()) return NULL;
        return array[0];
    }

    void pop() {
        if (empty()) return;

        pthread_mutex_lock(&mutex);
        if (array[0]) {
            delete array[0];
            array[0] = array[--cur_size];
            percolate_down(0);
        }
        pthread_mutex_unlock(&mutex);
    }

    void tick() {
        Timer *tmp = array[0];
        time_t cur = time(NULL);
        while (!empty()) {
            if (!tmp) break;
            if (tmp->expire > cur) break;

//          do timer task
            if (array[0]->cb_func) {
                array[0]->cb_func(array[0]->args);
            }

            pop();
            tmp = array[0];
        }
    }

    bool empty() const { return cur_size == 0; }

private:
    void percolate_down(int hole) {
        for (int child = (hole*2+1); child < cur_size; child = child*2+1) {
            if ((child+1) < cur_size && array[child+1]->expire < array[child]->expire) {
                ++child;
            }
            if (array[child]->expire < array[hole]->expire) {
                std::swap(array[hole], array[child]);
            } else {
                break;
            }
            hole = child;
        }
    }

    void resize() throw (std::exception) {
        Timer **tmp = new Timer *[2*capacity];
        for (int i = 0; i < 2 *capacity; ++i) {
            tmp[i] = NULL;
        }
        if (!tmp) {
            throw std::exception();
        }
        capacity = 2*capacity;
        for (int i = 0; i < cur_size; ++i) {
            tmp[i] = array[i];
        }
        delete [] array;
        array = tmp;
    }

private:
    Timer   **array;    /* time heap array */
    int     capacity;
    int     cur_size;
    /* lock heap when add, del timer */
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
};

#endif