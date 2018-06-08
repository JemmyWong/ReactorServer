#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h> /* rlimit */

#include "../include/slog.h"
#include "../include/server.h"
#include "../include/reactor.h"
#include "../include/configUtil.h"
#include "../include/thread_pool.h"
#include "../include/sig_handler.h"
#include "../include/eventhandler.h"

static int pipefd[2];
static threadPool_t *threadPool = NULL;
static char configPath[255] = "../config.ini";

int main(int argc, char **argv) {
    char key[20];
    char value[20];
    int ret, listenfd, epollfd;

    ret = slog_init();
    if (ret) {
        fprintf(stderr, "log_init error: %s\n", strerror(errno));
        exit(0);
    }
    printf("*************  Congratulations, ReactorServer begins **************\n");
    slog_info("*************  Congratulations, ReactorServer begins **************");


    // modify number of open file limits on linux
    strcpy(key, "RLIMIT");
    get_config(configPath, key, value);
    struct rlimit rl;
    rl.rlim_cur = atoi(value);
    rl.rlim_max = atoi(value);
    setrlimit(RLIMIT_NOFILE, &rl);
    getrlimit(RLIMIT_NOFILE, &rl);
    printf("Linux kernel rlimit, cur: %d, max: %d\n", (int)rl.rlim_cur, (int)rl.rlim_max);
    slog_info("Linux kernel rlimit, cur: %d, max: %d", (int)rl.rlim_cur, (int)rl.rlim_max);


/* init system */
    threadPool = threadPool_init(30);

    ret = init_server(&listenfd, &epollfd);
    if (ret) {
        fprintf(stderr, "init_server error: %s\n", strerror(errno));
        slog_error("init_server error: %s", strerror(errno));
        exit(0);
    }
    slog_info("server init success");

    // TODO pipefd is neve used
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);

    reactor_t *reactor = create_reactor(epollfd, threadPool);

/* create listen fd event handler */
    event_handler_t *acceptor = create_listen_handler(listenfd, reactor);
    if (acceptor == NULL) {
        fprintf(stderr, "acceptor is NULL\n");
    }
    reactor->add_eh(reactor, acceptor);
    slog_info("listen handler create and add to reactor");
    printf("create and add listen fd=%d to reactor, core_idx=%d\n",
           acceptor->fd, reactor->core->current_idx - 1);

/* create signal event handler */
    event_handler_t *sig_acceptor = create_signal_handler(reactor, SIGALRM);
    reactor->add_eh(reactor, sig_acceptor);
    alarm(TIMESLOT);
    printf("create and add signal fd=%d\n to reactor\n", sig_acceptor->fd);

    reactor->event_loop(reactor);

    return 0;
}

/* create listen fd and epoll fd */
int init_server(int *listenfd, int *epoolfd) {
    int ret = 0;
    char key[20];
    char value[20];
    struct sockaddr_in server;
    *listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*listenfd == -1) {
        fprintf(stderr, "create listen fd error: %s\n", strerror(errno));
        return -1;
    }

    /* after TCP 4 time disconnection, server on the time_wait status,
     * address and port can't be used util after 2MSL time */
    int opt = SO_REUSEADDR | SO_REUSEPORT;
    setsockopt(*listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(*listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    strcpy(key, "PORT");
    get_config(configPath, key, value);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(value));
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*listenfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "bind error: %s\n", strerror(errno));
        return -1;
    }

    strcpy(key, "BACKLOG");
    get_config(configPath, key, value);
    if (listen(*listenfd, atoi(value)) == -1) {
        fprintf(stderr, "listen error: %s\n", strerror(errno));
        return -1;
    }

    strcmp(key, "MAX_EPOLL");
    get_config(configPath, key, value);
    *epoolfd = epoll_create(atoi(value));
    if (*epoolfd == -1) {
        fprintf(stderr, "epoll_create error: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}