#include <unistd.h> /* alarm */

#include "../include/timer.h"
#include "../include/eventhandler.h"

int timerPipeFd[2] = {-1, -1};

static TimerHeap *timerHeap = new TimerHeap(10);

void timer_handle(void *arg) {
    printf("^^^^^^^^^ Timer ^^^^^^^^^^^^\n");
}

void addTimer(int delay) {
    Timer *timer = new Timer(delay);
    timer->cb_func = &timer_handle;
    timerHeap->add_timer(timer);
}

void handle_timer_event(handle_event_msg_t *handle_event_msg) {
    event_handler_t *eh = handle_event_msg->eh;
    uint32_t e = handle_event_msg->e;

    timerHeap->tick();
    // TODO
}

event_handler_t *create_timer_handler(reactor_t *reactor) {
    event_handler_t *eh = (event_handler_t *)malloc(sizeof(event_handler_t));
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, timerPipeFd);
    assert(ret != -1);

    eh->fd = timerPipeFd[0];
    eh->reactor = reactor;
    eh->handle_event = handle_timer_event;
    printf("timer heap top timer expire: %d\n", timerHeap->top()->expire);
    printf("call alarm(%d)\n", timerHeap->top()->expire + 2);
    alarm(timerHeap->top()->expire + 2);
    return eh;
}