#ifndef PROXY_HTTP_LOGGER_H
#define PROXY_HTTP_LOGGER_H
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include <buffer/buffer.h>
#include <selector/selector.h>
#include <buffer/buffer.h>
#include <logger/logger.h>

#define GET_LOG(key) (struct log*)((key)->data)

typedef enum {LOG_OK,PIPE_FAIL} log_status;

struct log{
    int readfd;
    int writefd;
    pthread_t logthread;
    buffer logbuf;
    void * log_buf_mem;
    fd_selector selector;
};

void log_close(struct selector_key * key);
void log_write(struct selector_key * key);


int log_start(struct log* l,fd_selector sel);

void log_send(struct log* l,char *s);
void log_sendf(struct log* l,const char *fmt, ...);

#endif //PROXY_HTTP_LOGGER_H
