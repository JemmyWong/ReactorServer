#ifndef _THREAD_POOL_
#define _THREAD_POOL_

#include <semaphore.h>
#include <pthread.h>
#include "Mutex.h"

typedef void (*FUNC)(void *arg);

typedef struct _thread_job_t {
    FUNC                    function;   /* call back function */
    void                    *arg;       /* function parameter */
    struct _thread_job_t    *prev;
    struct _thread_job_t    *next;
} thread_job_t;

typedef struct _thread_job_queue {
    int             number;     /* job number */
    sem_t           *queueSem;  /* control producer and consumer*/
    thread_job_t    *head;      /* queue head */
    thread_job_t    *tail;      /* queue tail */
} thread_job_queue;

/* thread pool */
typedef struct _threadPool_t {
    int                 number;
    pthread_t           *threads;
    thread_job_queue    *jobQueue;
} threadPool_t;

threadPool_t *threadPool_init(int number);

void threadPool_job_do(threadPool_t *pool);

void threadPool_add_work(threadPool_t *pool, void (*function)(void *), void *arg);

void threadPool_destroy(threadPool_t *pool);



int threadPool_jobQueue_init(threadPool_t *pool);

void threadPool_jobQueue_add(threadPool_t *pool, thread_job_t *newJob);

int threadPool_jobQueue_removeLast(threadPool_t *pool);

thread_job_t *threadPool_jobQueue_peek(threadPool_t *pool);

void threadPool_jobQueue_empty(threadPool_t *pool);

#endif