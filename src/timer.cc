#include "timer.h"
#include "Mutex.h"

TimerHeap::TimerHeap(int cap) throw(std::exception): capacity(cap), cur_size(0) {
    array = new Timer *[capacity];
    if (!array) {
        throw std::exception();
    }
    for (int i = 0; i < capacity; ++i) {
        array[i] = NULL;
    }
}
//  initialized with an existing array
TimerHeap::TimerHeap(Timer **init_array, int size, int cap) throw(std::exception)
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

TimerHeap::~TimerHeap() {
    for (int i = 0; i < cur_size; ++i) {
        delete array[i];
    }
    delete [] array;
}

void TimerHeap::add_timer(Timer *timer) throw(std::exception) {
    if (!timer) return;

    {
        MutexLockGuard lock(mutex);

        if (cur_size >= capacity) resize();

        int hole = cur_size++;
        int parent = 0;
        for (; hole > 0; hole = parent) {
            parent = (hole - 1) / 2;
            if (array[parent]->expire <= timer->expire)
                break;
            array[hole] = array[parent];
        }
        array[hole] = timer;
    }

    printf("new timer added, expire<%d>, heap capacity<%d>\n", timer->expire, capacity);
    slog_info("new timer added, expire<%d>, heap capacity<%d>", timer->expire, capacity);
}

//  delay destroy
void TimerHeap::del_timer(Timer *timer) {
    if (!timer) return;
//        timer->cb_func = NULL; TODO del_timer

    pthread_mutex_lock(&mutex);
    for (int i = (cur_size-1)/2; i >=0; --i) {
        percolate_down(i);
    }
    pthread_mutex_unlock(&mutex);
}

Timer *TimerHeap::top() const {
    if (empty()) return NULL;
    return array[0];
}

void TimerHeap::pop() {
    if (empty()) return;

    pthread_mutex_lock(&mutex);
    if (array[0]) {
        delete array[0];
        array[0] = array[--cur_size];
        percolate_down(0);
    }
    pthread_mutex_unlock(&mutex);
}

void TimerHeap::tick() {
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

void TimerHeap::percolate_down(int hole) {
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

void TimerHeap::resize() throw (std::exception) {
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