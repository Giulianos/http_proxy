#include <logger/logger.h>
#include <pthread.h>
#include <syslog.h>


int log_start(){
    openlog ("http_proxy", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);    return 42;

}

int log_write(char* msg){
    syslog (LOG_INFO, msg);    return 42;

}

int log_close(){
    closelog();
    return 42;
}