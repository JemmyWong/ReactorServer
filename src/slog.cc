#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <sys/types.h> /* getpid */
#include <unistd.h>

#include "slog.h"

static pthread_rwlock_t lock_env = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t lock_info = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t lock_error = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t lock_debug = PTHREAD_RWLOCK_INITIALIZER;
static pthread_key_t slog_thread_key;

static void slog_clean_thread_key() {
    slog_thread_t *slog_thread = static_cast<slog_thread_t *>(pthread_getspecific(slog_thread_key));
    if (slog_thread) free(slog_thread);
}

int slog_init() {
    printf("--------- slog init start -------------\n");

    int ret = pthread_rwlock_wrlock(&lock_env);
    if (ret) {
        printf("pthread_rwlock_wrlock error: %s\n", strerror(errno));
        return -1;
    }

    ret = pthread_key_create(&slog_thread_key, NULL);
    if (ret) {
        printf("pthread_key_create error: %s\n", strerror(errno));
        pthread_rwlock_unlock(&lock_env);
        return -1;
    }

    ret = atexit(slog_clean_thread_key);

    if (pthread_rwlock_unlock(&lock_env)){
        printf("pthread_rwlock_unlock error: %s\n", strerror(errno));
        return -1;
    }

    printf("----------- slog init end ----------\n");

    return 0;
}

#define fetch_thread_buffer(thread_buffer) do {\
    (thread_buffer) = (slog_thread_t *)pthread_getspecific(slog_thread_key);\
	if (!(thread_buffer)) {\
        (thread_buffer) = (slog_thread_t *)calloc(1, sizeof(slog_thread_t));\
		if (!(thread_buffer)) {\
			printf("fetch_thread_buffer calloc error\n");\
		}\
		int res = 0;\
		res = pthread_setspecific(slog_thread_key, thread_buffer);\
		if (res) {\
			printf("fetch_thread_buffer pthread_setspecific error \n");\
		}\
	}\
} while(0)

int slog(int level, const char *file, size_t filelen, const char *func,
         size_t funclen, long line, const char *format, ...) {
    slog_thread_t *thread_buffer = NULL;
    fetch_thread_buffer(thread_buffer);

    int ret;
    va_list arg;
    FILE *log_file_debug = NULL, *log_file_info = NULL, *log_file_error = NULL;

    va_start(arg, format);
    vsnprintf(thread_buffer->msg_buf, LOG_BUFFER_MAX_LENGTH, format, arg);
    va_end(arg);

    switch (level) {
        case SLOG_LEVEL_DEBUG: {
            ret = pthread_rwlock_wrlock(&lock_debug);
            if (ret) {
                slog_error("pthread_rwlock_wrlock error: %s", strerror(errno));
                return -1;
            }
            log_file_debug = fopen(LOG_FILE_DEBUG, "a");
            if (!log_file_debug) {
                slog_error("open file <%s> error: %s", LOG_FILE_DEBUG, strerror(errno));
                return -1;
            }
            fprintf(log_file_debug, "DEBUG: %s (pid: %d, %s:%d, %s %s)\n",
                    thread_buffer->msg_buf, getpid(), func, line,__TIME__,  __DATE__);
            fclose(log_file_debug);
            ret = pthread_rwlock_unlock(&lock_debug);
            if (ret) {
                slog_error("pthread_rwlock_unlock error: %s", strerror(errno));
            }
            break;
        }
        case SLOG_LEVEL_INFO: {
            ret = pthread_rwlock_wrlock(&lock_info);
            if (ret) {
                slog_error("pthread_rwlock_wrlock error: %s", strerror(errno));
                return -1;
            }
            log_file_info = fopen(LOG_FILE_INFO, "a");
            if (!log_file_info) {
                slog_error("open file<%s> error: %s", LOG_FILE_INFO, strerror(errno));
                return -1;
            }
            fprintf(log_file_info, "INFO: %s (pid: %d, %s: %d, %s %s)\n",
                thread_buffer->msg_buf, getpid(), func, line,__TIME__,  __DATE__);
            fclose(log_file_info);
            ret = pthread_rwlock_unlock(&lock_info);
            if (ret) {
                slog_error("pthread_rwlock_unlock error: %s", strerror(errno));
            }
            break;
        }
        case SLOG_LEVEL_ERROR: {
            ret = pthread_rwlock_wrlock(&lock_error);
            if (ret) {
                slog_error("pthread_rwlock_wrlock error: %s", strerror(errno));
                return -1;
            }
            log_file_error = fopen(LOG_FILE_ERROR, "a");
            if (!log_file_error) {
                slog_error("open file<%s> error: %s", LOG_FILE_ERROR, strerror(errno));
                return -1;
            }
            fprintf(log_file_error, "ERROR: %s (pid: %d, %s: %d, %s %s])\n",
                    thread_buffer->msg_buf, getpid(), func, line, __TIME__,  __DATE__);
            fclose(log_file_error);
            ret = pthread_rwlock_unlock(&lock_error);
            if (ret) {
                slog_error("pthread_rwlock_unlock error: %s", strerror(errno));
            }
            break;
        }
        default:
            slog_error("error slog tag");
    }
    return 0;
}