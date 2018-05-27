#include <logger/logger.h>



void log_start(){
    openlog ("httpd-proxy", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

}

void log_info_write(char* msg){
    syslog (LOG_INFO, msg); //TODO: no bloqueante
}

void log_err_write(char* msg){
    syslog (LOG_ERR, msg); //TODO: no bloqueante
}

void log_close(){
    closelog();
}