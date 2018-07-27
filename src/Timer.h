#ifndef _TIMER_H_
#define _TIMER_H_

#include <iostream>
#include <netinet/in.h>
#include <ctime>
#include <cassert>
#include <pthread.h>

#include <algorithm>
#include <utility>
/* swap */

#include "Slog.h"
#include "Mutex.h"

using std::exception;

#define BUFFER_SIZE 64

class Timer;

/*void handle_timer_event(handle_event_msg_t *handle_event_msg);

event_handler_t *create_timer_handler(reactor_t *reactor);

*//* provided for handle_timer.cc *//*

void addTimer(int delay);

struct ClientData {
    int         fd;
    Timer       *timer;
    sockaddr_in address;
    char buf[BUFFER_SIZE];
};*/

class Timer {
public:
    explicit Timer(int delay) {
        args = nullptr;
        cb_func = nullptr;
        expire = delay;
    }

public:
    void *args;
//    time_t expire;
    int expire;
    void (*cb_func)(void *);
};

class TimerHeap {
public:
    explicit TimerHeap(int cap) throw(std::exception);
//  initialized with an existing array
    TimerHeap(Timer **init_array, int size, int cap) throw(std::exception);

    ~TimerHeap();

public:
    void add_timer(Timer *timer) throw(std::exception);

//  delay destroy
    void del_timer(Timer *timer);

    Timer *top() const;

    void pop();

    void tick();

    bool empty() const { return cur_size == 0; }

private:
    void percolate_down(int hole);

    void resize() throw (std::exception);
private:
    Timer   **array;
/* time heap array */
    int     capacity;
    int     cur_size;

/* lock heap when add, del timer */
//    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    MutexLock mutex;
};

#endif
