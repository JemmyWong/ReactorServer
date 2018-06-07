#include "../include/slog.h"
#include "../include/header.h"
#include "../include/reactor.h"
#include "../include/server.h"
#include "../include/thread_pool.h"
#include "../include/global.h"

#include <signal.h>

static int pipefd[2];

void sig_handler(int sig) {
    // TODO why save previous errno?
    int signal = sig;
    int save_error = errno;
    send(pipefd[1], &signal, sizeof(sig), 0);
    errno = save_error;
}

/*
 * struct sigaction {
      void (*sa_handler)(int); // SIG_IGN, SIG_DFL
      void (*sa_sigaction)(int, siginfo_t *, void *);   // si_code, si_signo, si_value
      sigset_t  sa_mask;
      int       sa_flags; // SA_NOCLDSTOP, SA_NOCLDWAIT, SA_NODEFER, SA_RESTART
      void (*sa_restorer)(void);
    };
 *
 * */

void addsig(int sig) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);

    printf("add signal = %d\n", sig);
}

static void handle_signal_event(handle_event_msg_t *handle_event_msg) {
    event_handler_t *eh = handle_event_msg->eh;
    uint32_t e = handle_event_msg->e;
    int ret, sig;
    ret = recv(eh->fd, &sig, sizeof(sig), 0);
    printf("handle signal event <%d>\n", sig);
    if (ret <= 0) {
        fprintf(stderr, "handle_signal_event recv error: %s\n", strerror(errno));
    } else {
        switch (sig) {
            case SIGALRM:
//                alarm(TIMESLOT);
                printf("signal alarm\n");
                slog_info("handel_signal_event of SIGALRM");
                break;
            case SIGINT:
                printf("signal interupt\n");
                exit(0);
        }
    }
}

event_handler_t *create_signal_handler(reactor_t *reactor, int sig) {
    event_handler_t *eh = (event_handler_t *)malloc(sizeof(event_handler_t));
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    eh->fd = pipefd[0];
    eh->reactor = reactor;
    eh->handle_event = &handle_signal_event;
    addsig(sig);

    printf("create signal handler <%d>\n", sig);

    return eh;
}