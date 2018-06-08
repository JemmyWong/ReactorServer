#include "../include/slog.h"
#include "../include/global.h"
#include "../include/thread_pool.h"
#include "../include/reactor.h"

static int threadPool_keepAlive = 1;

/* serialize queue access*/
/* initialization will not execute immediately until first call */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * data members of threadPool_t must be initialized in correct order,
 * otherwise, segment fault
 */
threadPool_t *threadPool_init(int number) {
    slog_info("- - - - - - - - - - - - - -  init threadPool - - - - - - -- - - - - - - - -\n");
    if (number < 1) number = 20;
    if (number > 50) number = 50;
    slog_info("threadPool number is <%d>", number);

    threadPool_t *threadPool = (threadPool_t *)malloc(sizeof(threadPool_t));
    if (!threadPool) {
        printf("malloc threadPool_t error: %s\n", strerror(errno));
        slog_error("malloc threadPool_t error: %s", strerror(errno));

        free(threadPool);
        return NULL;
    }

    pthread_t *threads = (pthread_t *)malloc(number * sizeof(pthread_t));
    if (!threads) {
        printf("malloc pthread_t[] error: %s\n", strerror(errno));
        slog_error("malloc pthread_t[] error: %s", strerror(errno));

        free(threadPool);
        return NULL;
    }
    threadPool->number = number;
    threadPool->threads = threads;

    if (threadPool_jobQueue_init(threadPool)) {
        printf("threadPool_jobQueue_init error: %s\n", strerror(errno));
        slog_error("threadPool_jobQueue_init error: %s", strerror(errno));

        free(threads);
        free(threadPool);
        return NULL;
    }
    threadPool->jobQueue->queueSem = (sem_t *)malloc(sizeof(sem_t));
    sem_init(threadPool->jobQueue->queueSem, 0, 0);
    slog_info("threadPool jobQueue init finished");

    int i;
    for (i = 0; i < number; ++i) {
        pthread_create(&(threads[i]), NULL, (void* (*)(void*))threadPool_job_do, (void *)threadPool);
    }
    slog_info("threadPool threads creation finished");

    int semN;
    sem_getvalue(threadPool->jobQueue->queueSem, &semN);
    printf("thread_pool init success, number=%d, sem=%d\n", threadPool->number, semN);
    slog_info("thread_pool init success, number=%d, sem=%d", threadPool->number, semN);
    slog_info("- - - - - - - - - - - - - -  init threadPool end - - - - - - -- - - - - - -\n");

    return threadPool;
}

void threadPool_destroy(threadPool_t *threadPool) {
    slog_info("- - - - - - - - - - - - - -  destroy threadPool - - - - - - -- - - - - - -\n");
    if (!threadPool) {
        slog_info("threadPool is empty, return directly");
        return;
    }

    threadPool_keepAlive = 0;
    int i;
    for (i = 0; i < threadPool->number; ++i) {
        if (sem_post(threadPool->jobQueue->queueSem)) {
            fprintf(stderr, "could not bypass sem_wait(), error: %s\n", strerror(errno));
            slog_error("could not bypass sem_wait(), error: %s", strerror(errno));
        }
    }
    // TODO threadPool destory
    if (sem_post(threadPool->jobQueue->queueSem) != 0) {
        fprintf(stderr, "could not destroy semaphore, error: %s\n", strerror(errno));
        slog_error("could not destroy semaphore, error: %s", strerror(errno));
    }
    slog_info("waiting for threads finish jobs");
    for (i = 0; i < threadPool->number; ++i) {
        pthread_join(threadPool->threads[i], NULL);
    }
    slog_info("all threads finish jobs");
    threadPool_jobQueue_empty(threadPool);
    free(threadPool->threads);
    free(threadPool->jobQueue->queueSem);
    free(threadPool->jobQueue);
    free(threadPool);

    slog_info("- - - - - - - - - - - - - -  destroy threadPool end - - - - - - -- - - - - - -\n");

}

void threadPool_add_work(threadPool_t *threadPool, void (*function)(void *), void *arg) {
    thread_job_t *newJob = (thread_job_t *)malloc(sizeof(thread_job_t));
    newJob->function = function;
    newJob->arg = arg;

    pthread_mutex_lock(&mutex);
    threadPool_jobQueue_add(threadPool, newJob);
    slog_info("add new job to jobQueue, fd<%d>", ((handle_event_msg_t *)arg)->eh->fd);
    pthread_mutex_unlock(&mutex);

    // TODO segment fault
//    printf("new job add to jobQueue, fd=%d\n", ((handle_event_msg_t *)arg)->eh->fd);
}

void threadPool_job_do(threadPool_t *threadPool) {
    while (threadPool_keepAlive) {
        /* thread will be blocked, waiting for notification until
         * there is a work in jobQueue */
        if (sem_wait(threadPool->jobQueue->queueSem)) {
            fprintf(stderr, "sem_wait error: %s\n", strerror(errno));
            exit(1);
        }
        if (threadPool_keepAlive) {
            FUNC function;
            void *args;
            thread_job_t *job;
            pthread_mutex_lock(&mutex);
            job = threadPool_jobQueue_peek(threadPool);
            function = job->function;
            args = job->arg;
            /* TODO mutex will not unblock? */
            /* what happened to the thread when it exit this function, died? */
            if (threadPool_jobQueue_removeLast(threadPool)) {
                pthread_mutex_unlock(&mutex);
                return;
            }
            pthread_mutex_unlock(&mutex);
            printf("thread[%d] get a job\n", pthread_self());
            slog_info("thread<%d> get a job", pthread_self());
            function((handle_event_msg_t *)args);
            printf("thread[%d] finish job\n\n\n", pthread_self());
            slog_info("thread[%d] finish job\n\n\n", pthread_self());
            free(job);
        }
    }
}

/* jobQueue */
int threadPool_jobQueue_init(threadPool_t *pool) {
    thread_job_queue *jobQueue = (thread_job_queue *)malloc(sizeof(thread_job_queue));
    if (!jobQueue) {
        fprintf(stderr, "malloc thread_job_queue error: %s\n", strerror(errno));
        return -1;
    }

    jobQueue->number = 0;
    jobQueue->head = NULL;
    jobQueue->tail = NULL;
    pool->jobQueue = jobQueue;
    slog_info("threadPool joQueue inited");
    return 0;
}

void threadPool_jobQueue_add(threadPool_t *pool, thread_job_t *newJob) {
    newJob->next = NULL;
    newJob->prev = NULL;
    switch (pool->jobQueue->number) {
        case 0:
            pool->jobQueue->head = newJob;
            pool->jobQueue->tail = newJob;
            break;
        default:
            newJob->next = pool->jobQueue->head;
            pool->jobQueue->head->prev = newJob;
            pool->jobQueue->head = newJob;
    }
    ++(pool->jobQueue->number);

    sem_post(pool->jobQueue->queueSem);

    int retVal;
    sem_getvalue(pool->jobQueue->queueSem, &retVal);
    printf("new job add to jobQueue, sem_number=%d\n", retVal);
    slog_info("new job add to jobQueue, sem_number=%d", retVal);

}

int threadPool_jobQueue_removeLast(threadPool_t *pool) {
    if (!pool) return -1;

    thread_job_t *lastJob = pool->jobQueue->tail;
    switch (pool->jobQueue->number) {
        case 0:
            return -1;
        case 1:
            pool->jobQueue->head = NULL;
            pool->jobQueue->tail = NULL;
            break;
        default:
            pool->jobQueue->tail = lastJob->prev;
            lastJob->prev->next = NULL;
    }
    --(pool->jobQueue->number);
//    free(lastJob); /* last job freed after finish*/

    int retVal;
    sem_getvalue(pool->jobQueue->queueSem, &retVal); // TODO which purpose?
    printf("last job remove from jobQueue, sem_number=%d\n", retVal);
    slog_info("last job remove from jobQueue, sem_number=%d", retVal);
    return 0;
}

thread_job_t *threadPool_jobQueue_peek(threadPool_t *pool) {
    return pool->jobQueue->tail;
}

void threadPool_jobQueue_empty(threadPool_t *pool) {
    slog_info("************* jobQueue destroy ****************");
    thread_job_t *curJob = pool->jobQueue->tail;

    while (pool->jobQueue->number > 0) {
        pool->jobQueue->tail = curJob->prev;
        free(curJob);
        curJob = pool->jobQueue->tail;
        --(pool->jobQueue->number);
    }

    pool->jobQueue->head = NULL;
    pool->jobQueue->tail = NULL;

    slog_info("************* jobQueue destroy end ****************");
}
