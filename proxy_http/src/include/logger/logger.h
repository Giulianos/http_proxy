#ifndef PROXY_HTTP_LOGGER_H
#define PROXY_HTTP_LOGGER_H
#define LOG_FILE "proxy.log"

#include <fcntl.h>


int log_open();

int log_write(char* msg);

int log_close();

#endif //PROXY_HTTP_LOGGER_H
