#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include "slog.h"
#include "reactor.h"
#include "commonUtil.h"
#include "header.h"
#include "reactor.h"
#include "configUtil.h"
#include "Mutex.h"

static char configPath[255] = "../config.conf";
//pthread_mutex_t mutex_eh = PTHREAD_MUTEX_INITIALIZER;
MutexLock mutex_eh;

event_handler_t *find_eh(reactor_core_t *core, int fd, int *index) {
    int i = 0;
    event_handler_t *tmp;
    {
        MutexLockGuard lock(mutex_eh);
        for (; i < core->current_idx; ++i) {
            if ((tmp = core->ehs[i]) && (tmp->fd == fd)) {
                if (index) *index = i;
                printf("find_eh, fd[%d], index[%d]\n", fd, i);
                slog_info("find_eh, fd[%d], index[%d]", fd, i);
                return tmp;
            }
        }
        printf("not found event_handler which fd:[%d]\n", fd);
        slog_error("not found event_handler which fd:[%d]", fd);
    }

    return NULL;
}

int add_eh(reactor_t *reactor, event_handler_t *eh) {
    int result = 0;
    struct epoll_event ee;

    if (!eh || eh->fd < 0 || eh->handle_event == NULL) {
        printf("fd < 0 or handle_event is NULL\n");
        result = -1;
    }
    {
        MutexLockGuard lock(mutex_eh);
        if (reactor->core->current_idx < MAX_USER) {
//        ee.data.fd = eh->fd;
//        ee.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
//        epoll_ctl(reactor->core->epoll_fd, EPOLL_CTL_ADD, eh->fd, &ee);
            addFd(reactor->core->epoll_fd, eh->fd, true);

            reactor->core->ehs[reactor->core->current_idx++] = eh;
            printf("add event to reactor, core current idx:[%d], fd:[%d]\n",
                   (int)reactor->core->current_idx-1, eh->fd);
        } else {
            result = -1;;
        }
    }

    return result;
}

int rm_eh(reactor_t *reactor, int fd) {
    int position;
    event_handler_t *eh = find_eh(reactor->core, fd, &position);
    if (!eh) {
        printf("rm_eh: can not find event_handler which fd:[%d]\n", fd);
        return -1;
    }
    if (position >= reactor->core->current_idx) {
        printf("invalid event_handler index:[%d]\n", position);
        return -2;
    }
    {
        MutexLockGuard lock(mutex_eh);
        reactor->core->ehs[position] = reactor->core->ehs[reactor->core->current_idx--];
        printf("remove event handler from reactor, core current idx:[%d], fd:[%d]\n",
               (int)reactor->core->current_idx-1, fd);
        slog_info("rm event handler, core current idx:[%d], fd:[%d]",
                  (int)reactor->core->current_idx-1, fd);
    }

//    epoll_ctl(reactor->core->epoll_fd, EPOLL_CTL_DEL, fd, 0);
    removeFd(reactor->core->epoll_fd, fd);

//    close(eh->fd);
    free(eh);
}

int event_loop(reactor_t *reactor) {
    printf("start event_loop .....  \n");
    slog_info("start event_loop .....");

    int i, num;
    char key[20];
    char value[20];
    strcpy(key, "MAX_EPOLL_EVENT");
    get_config(configPath, key, value);
    struct epoll_event ees[atoi(value)];
    event_handler_t *eh= NULL;
    while (1) {
        num = epoll_wait(reactor->core->epoll_fd, ees, atoi(value), -1);
        printf("epoll_wait return, detect event number is %d\n", num);
        slog_info("epoll_wait return, detect event number is %d", num);

        for (i = 0; i < num; ++i) {
            if (ees[i].events & EPOLLRDHUP) {
                // TODO implement user offline operation
                // user_offline(ees[i].data.fd);
                printf("user down\n");
                slog_info("user down, fd<%d>", ees[i].data.fd);
//                rm_eh(reactor, ees[i].data.fd);
            } else {
                eh = find_eh(reactor->core, ees[i].data.fd, 0);
                if (eh) {
                    handle_event_msg_t *handle_event_msg = (handle_event_msg_t *)malloc(sizeof(handle_event_msg_t));
                    handle_event_msg->eh = eh;
                    handle_event_msg->e = ees[i].events;

                    threadPool_add_work(reactor->threadPool, (void (*)(void*))eh->handle_event, (void *)handle_event_msg);
                } else {
                    printf("event_loop find_eh eh is NULL\n");
                    slog_error("event_loop find_eh eh is NULL, fd<%d>\n", ees[i].data.fd);
                }
            }
        }
    }
}

reactor_t *create_reactor(int epollfd, threadPool_t *threadPool) {
    reactor_core_t *core = (reactor_core_t *)malloc(sizeof(reactor_core_t));
    core->epoll_fd = epollfd;
    core->current_idx = 0;
    core->ehs[0] = NULL;

    reactor_t *reactor = (reactor_t *)malloc(sizeof(reactor_t));
    reactor->add_eh = &add_eh;
    reactor->rm_eh = &rm_eh;
    reactor->event_loop = &event_loop;
    reactor->core = core;
    reactor->threadPool = threadPool;

    return reactor;
}




