#ifndef _SIG_HANDLER_H_
#define _SIG_HANDLER_H_

void sig_handler(int sig);

void addsig(int sig);

static void handle_signal_event(handle_event_msg_t *handle_event_msg);

event_handler_t *create_signal_handler(reactor_t *reactor, int sig);


#endif