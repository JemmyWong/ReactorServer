#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* sleep */
#include <sys/socket.h>
#include <netdb.h> /* hostent */
#include <errno.h>

#include "../src/configUtil.h"
#include "../src/HttpConn.h"
#include "../src/commonUtil.h"


#define PORT 9000
#define MAX_DATA_SIZE   1024

static int totalConn = 0;

void *send_sock(void *arg) {
    int fd = *(reinterpret_cast<int *>(arg));
    char msg[1024] = "Hi, Server. I'm Client";
    if (send(fd, msg, strlen(msg), 0) == -1) {
        fprintf(stderr, "send_sock errror: %s\n", strerror(errno));
        exit(1);
    }
    if (recv(fd, msg, MAX_DATA_SIZE, 0) == -1) {
        fprintf(stderr, "recv from server error: %s\n", strerror(errno));
    } else {
        printf("recv form server: %s\n", msg);
        printf("++++++++++   ++++++++++++++++   totalConn: <%d>\n\n", ++totalConn);
    }
//    close(fd);
}

void *sendHttpRequest(void *arg) {
    int fd = *(reinterpret_cast<int *>(arg));
    char * request = "GET /index.html HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 10\r\n\r\nabcddfesdf";
    if (send(fd, request, strlen(request), 0) == -1) {
            fprintf(stderr, "send_sock errror: %s\n", strerror(errno));
            exit(1);
    }
    char buf[4096];
    memset(buf, '\0', sizeof(buf));
    if (recv(fd, buf, 4096, 0) == -1) {
        fprintf(stderr, "recv from server error: %s\n", strerror(errno));
    } else {
        printf("recv form server: %s\n", buf);
        printf("++++++++++   ++++++++++++++++   totalConn: <%d>\n\n", ++totalConn);
    }

}


void start_thread_pool(void *arg) {
    pthread_t thds[1];
    int i;
    for (i = 0; i < 1; ++i) {
        pthread_create(&thds[i], NULL, sendHttpRequest, arg);
    }
    for (i = 0; i < 1; ++i) {
        pthread_join(thds[i], 0);
    }
}

void *threadMain(void *arg) {
    int sockfd, num;
    char buf[MAX_DATA_SIZE];
    struct hostent *he;
    struct sockaddr_in server;
    char key[20];
    char value[20];

    if ((he = gethostbyname("127.0.0.1")) == NULL) {
        fprintf(stderr, "gethostbyname error: %s\n", strerror(errno));
        exit(-1);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "socket error: %s\n",strerror(errno));
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr = *((struct in_addr *)he->h_addr);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "connect error: %s\n", strerror(errno));
        exit(1);
    }

    start_thread_pool((void *)&sockfd);

    return 0;
}

void *multiSingleThread(void *arg) {
    int i = 0;
    int sockfd[10], num;
    char buf[MAX_DATA_SIZE];
    struct hostent *he;
    struct sockaddr_in server;

    if ((he = gethostbyname("127.0.0.1")) == NULL) {
        fprintf(stderr, "gethostbyname error: %s\n", strerror(errno));
        exit(-1);
    }
    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr = *((struct in_addr *)he->h_addr);


    for (i = 0; i < 10; ++i) {
        if ((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            fprintf(stderr, "socket error: %s\n",strerror(errno));
            exit(1);
        }

        if (connect(sockfd[i], (struct sockaddr *)&server, sizeof(server)) == -1) {
            fprintf(stderr, "connect error: %s\n", strerror(errno));
            exit(1);
        }

        int fd = sockfd[i];
        char msg[1024] = "Hi, Server. I'm Client";
        if (send(fd, msg, strlen(msg), 0) == -1) {
            fprintf(stderr, "send_sock errror: %s\n", strerror(errno));
            exit(1);
        }
        if (recv(fd, msg, 1024, 0) == -1) {
            fprintf(stderr, "recv from server error: %s\n", strerror(errno));
        } else {
            printf("recv form server: %s\n", msg);
            printf("++++++++++   ++++++++++++++++   totalConn: <%d>\n\n", ++totalConn);
        }
    }

    sleep(5);
    for (i = 0; i < 10; ++i) {
        close(sockfd[i]);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: enter thread number\n");
        return 0;
    }
    pthread_t thds[65530];
    int i;
    int n = atoi(argv[1]);
    for (i = 0; i < n; ++i) {
        pthread_create(&thds[i], NULL, threadMain, NULL);
    }
    for (i = 0; i < n; ++i) {
        pthread_join(thds[i], 0);
    }

    return 0;
}
