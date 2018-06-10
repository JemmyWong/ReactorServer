#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* basic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* strerror, memset */
#include <unistd.h> /* alarm */
#include <signal.h>
#include <assert.h>
#include <errno.h> /* linux c error code */

/* internet */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* thread */
#include <pthread.h>
#include <semaphore.h>
#include <sys/epoll.h>

#endif