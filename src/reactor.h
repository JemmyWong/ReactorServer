#ifndef _REACTOR_H_
#define _REACTOR_H_

#include "global.h"
#include "eventhandler.h"
#include "thread_pool.h"

#define MAX_USER    65530

typedef struct reactor_core {
    int epoll_fd;
    size_t  current_idx;
    event_handler_t *ehs[MAX_USER-1];
} reactor_core_t;

typedef struct reactor {
    int (*add_eh)(struct reactor *self, event_handler_t *eh);
    int (*rm_eh)(struct reactor *self, int fd);
    int (*event_loop)(struct reactor *);
    reactor_core_t *core;
    threadPool_t *threadPool;
} reactor_t;


reactor_t *create_reactor(int fd, threadPool_t *);

event_handler_t *find_eh(reactor_core_t *core, int fd, int *index);

int event_loop(reactor_t *reactor);

int rm_eh(reactor_t *reactor, int fd);

int add_eh(reactor_t *reactor, event_handler_t *eh);

#endif