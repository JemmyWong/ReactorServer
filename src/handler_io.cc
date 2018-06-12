#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <error.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "reactor.h"
#include "thread_pool.h"
#include "header.h"
#include "eventhandler.h"

int recv_from_fd(int socket_fd, char *recv_buf, int *recv_len) {
    int rev = -1;
    int one_time_need_read = 1024, one_time_recv_len;
    int continue_read = 1;
    int total_recv_len = 0;
    while (continue_read) {
        one_time_recv_len = recv(socket_fd, &recv_buf[total_recv_len],
                one_time_need_read, 0);
        if (one_time_recv_len > 0) {
            total_recv_len += one_time_recv_len;
        }

        if (one_time_recv_len < 0) { // recv error
//			if (errno == EAGAIN)
//			{
//				continue_read = 0;
//				break;
//			}
            continue_read = 0;
            rev = -1;
            break;
        } else if (one_time_recv_len >= 0 && one_time_recv_len < one_time_need_read) { // recv over
            continue_read = 0;
            rev = 0;
            *recv_len = total_recv_len;
            break;
        } else { // peer closed
            continue_read = 0;
            rev = -1;
            break;
        }
    }
    return rev;
}


int send_to_fd(int socket_fd, char *send_buf, int send_len) {
    int nwrite;
    int n = send_len; // bytes of data need to be sent

    while (n > 0) {
        nwrite = write(socket_fd, send_buf + send_len - n, n);
        if (nwrite < n) {
            if (nwrite == -1) {
                if (errno != EAGAIN) {
                    printf("send_to_fd<%d> again: %s\n", socket_fd, strerror(errno));
                    return 0;
                } else {
                    printf("send_to_fd<%d> error: %s\n", socket_fd, strerror(errno));
//                    snprintf(log_str_buf, LOG_STR_BUF_LEN, "Write EAGAIN.\n");
//                    LOG_INFO(LOG_LEVEL_INFO, log_str_buf);
                }
            }
        }
        n -= nwrite;
    }
    return (send_len - n);
}


/* handle client fd event */
void handle_client_event(handle_event_msg_t *handle_event_msg) {
    event_handler_t *eh = handle_event_msg->eh;
    int e = handle_event_msg->e;
    int ret, recv_len;
    common_package_t *package = (common_package_t *)malloc(sizeof(common_package_t));
    receive_msg_t *receive_msg = (receive_msg_t *)malloc(sizeof(receive_msg_t));

    printf("handle_client_evnet now............\n");

    char msgBuf[4096];
    if (e & EPOLLRDHUP) {
        // TODO　thread safe
//        eh->reactor->rm_eh(eh->reactor, eh->fd);
        close(eh->fd);
        printf("client down\n");
    } else if (e & EPOLLIN){
//        ret = recv(eh->fd, package, sizeof(common_package_t), 0);
//        printf("recv from client success\n");
//        if (ret < 0) {
//            printf("receive from fd[%d] error: %s\n", eh->fd, strerror(errno));
//            close(eh->fd);
//        } else {
//            printf("receive body: %s\n", package->body);
//            strcpy(package->body, "This is a server response message");
//            ret = send(eh->fd, package, sizeof(common_package_t), 0);
//
//            // receive_msg->fd = self->fd;
//            // strcpy(receive_msg->ip,"127.0.0.1");
//            // reveive_message(receive_msg);
//        }
//        ret = recv(eh->fd, msgBuf, 4096, 0);


        /*ret = recv_from_fd(eh->fd, msgBuf, &recv_len);
        if (recv_len > 4096) {
            printf("error recv from fd<%d> buffer overflow, recv_len: %d\n", eh->fd, recv_len);
        }
        if (ret < 0) {
            printf("receive from client fd[%d] error: %s\n", eh->fd, strerror(errno));
//            TODO close socket and remove form epoll
//            close(eh->fd);
        } else {
            printf("receive msg: %s\n", msgBuf);
            printf("core_idx: %d\n", (int)eh->reactor->core->current_idx - 1);
            strcpy(msgBuf, "This is a server response message");
//            ret = send(eh->fd, msgBuf, sizeof(msgBuf), 0);
            ret = send_to_fd(eh->fd, msgBuf, (int)strlen(msgBuf));
        }

        free(package);
        free(receive_msg);
        free(handle_event_msg);*/


        if (eh->httpConn->read()) {
            eh->httpConn->process();
//            eh->httpConn->write();
        } else {
            eh->httpConn->closeConn();
            delete(eh->httpConn);
        }
    } else if (e & EPOLLOUT) {
        if (!eh->httpConn->write()) {
            eh->httpConn->closeConn();
            delete(eh->httpConn);
        }

    }
}

/* create client fd event_handler */
event_handler_t *create_client_handler(int fd, reactor_t *reactor) {
    event_handler_t *eh = (event_handler_t *)malloc(sizeof(event_handler_t));
    eh->fd = fd;
    eh->reactor = reactor;
    eh->handle_event = handle_client_event;

    printf("create client event handler, fd = %d\n", fd);

    return eh;
}

/* handle listen fd event */
void handle_listen_event(handle_event_msg_t *handle_event_msg) {
    event_handler_t *eh = handle_event_msg->eh;
    int e = handle_event_msg->e;

    struct sockaddr_in cli_addr;
    socklen_t  cli_adrr_len = sizeof(cli_addr);
    memset(&cli_addr, 0, sizeof(cli_addr));

    /* accept client connection */
    int cli_fd = accept(eh->fd, (struct sockaddr *)&cli_addr, &cli_adrr_len);

    /* listen client event handler */
    event_handler_t *ceh = create_client_handler(cli_fd, eh->reactor);
    ceh->httpConn = new HttpConn();
    ceh->httpConn->init(cli_fd, cli_addr);

    eh->reactor->add_eh(eh->reactor, ceh);
    printf("accept client event fd: %d\n", cli_fd);

    free(handle_event_msg);
}

/* create listen fd event_handler */
event_handler_t *create_listen_handler(int fd, reactor_t *reactor) {
    printf("create event_handler of fd[%d]\n", fd);

    event_handler_t *eh = (event_handler_t *)malloc(sizeof(event_handler_t));
    eh->fd = fd;
    eh->reactor = reactor;
    eh->handle_event = handle_listen_event;

    return eh;
}



