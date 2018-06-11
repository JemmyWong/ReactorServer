#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include <stdint.h> /* uint32_t */

/*
 * event handler:
 * 1. listen socket event
 * 2. client socket event
 * 3. timer event
 */


typedef struct reactor reactor_t;
typedef struct handle_event_msg handle_event_msg_t;

/* Node for the event handler of a socket fd */
typedef struct event_handler {
    int fd;
    struct reactor *reactor;
    void (*handle_event)(handle_event_msg_t *handle_event_msg);
} event_handler_t;

typedef struct handle_event_msg {
    uint32_t e;             /* epoll_event.events */
    event_handler_t *eh;
} handle_event_msg_t;

int recv_from_fd(int sock_fd, char *buf, int *len);

int send_to_fd(int sock_fd, char *buf, int len);

void handle_client_event(handle_event_msg_t *handle_event_msg);

event_handler_t *create_client_handler(int fd, reactor_t *reactor);

void handle_listen_event(handle_event_msg_t *handle_event);

event_handler_t *create_listen_handler(int fd, reactor_t *reactor);

#endif
