#ifndef _LOG_H_
#define _LOG_H_

#include <cstddef> /* size_t */

#define LOG_BUFFER_MAX_LENGTH 1024
#define LOG_FILE_DEBUG   "debug.log"
#define LOG_FILE_INFO    "info.log"
#define LOG_FILE_ERROR   "error.log"
#define LOG_VERSION      "1.0"


#define SLOG_LEVEL_DEBUG    20
#define SLOG_LEVEL_INFO     40
#define SLOG_LEVEL_ERROR    60

typedef struct {
    int init_version;
    char msg_buf[LOG_BUFFER_MAX_LENGTH + 1];
} slog_thread_t;

int slog_init();

// int fetch_thread_buffer(slog_thread_t **thread_buffer);
int slog(int level, const char *file, size_t filelen, const char *func,
    size_t funclen, long line, const char *format, ...);

#define slog_debug(...)\
    slog(SLOG_LEVEL_DEBUG, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1,\
    __LINE__, __VA_ARGS__)

#define slog_info(...)\
    slog(SLOG_LEVEL_INFO, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1,\
    __LINE__, __VA_ARGS__)

#define slog_error(...)\
	slog(SLOG_LEVEL_ERROR, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1,\
    __LINE__, __VA_ARGS__)

#endif