#ifndef PROXY_HTTP_LOGGER_H
#define PROXY_HTTP_LOGGER_H
#define LOG_FILE "proxy.log"

#include <fcntl.h>
#include <pthread.h>
#include <syslog.h>

void log_open();

void log_err_write(char* msg);

void log_info_write(char* msg);

void log_close();

#endif //PROXY_HTTP_LOGGER_H
