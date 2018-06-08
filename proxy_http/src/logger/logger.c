#include <logger/logger.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>


void log_thread(struct log* l);

int log_start(struct log* l,fd_selector s){
    int fd[2];
    if(pipe(fd)==-1)
        return PIPE_FAIL;
    l->readfd=fd[0];
    l->writefd=fd[1];
    l->log_buf_mem=malloc(8192);
    l->selector=s;
    buffer_init(&l->logbuf, 8192, l->log_buf_mem);
    openlog ("httpd-proxy", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    pthread_create(&l->logthread,NULL,( void * (*)(void *)) &log_thread,(void *)l);
    return LOG_OK;
}

void log_thread(struct log* l){
    __uint8_t cache[2048];
    int position=0;
    while(true) {
        int c =255;
        int state= (int) read(l->readfd, &c, 1);
        if (state == 0)
            break;
        if (state==-1){
            continue;
        }
        cache[position]= (__uint8_t) c;
        position++;
        if ((position<2046 && c==0) || position==2046){
            syslog(LOG_INFO, (const char *) cache);
            memset(cache,0,2048);
            position=0;
        }
    }
}

void log_send(struct log* l,char *s){
    size_t len;
    len = strlen(s);
    if(len>=2047){
        perror("Too long");
        return;
    }
    size_t buffer_space;
    uint8_t * buffer_ptr = buffer_write_ptr(&l->logbuf, &buffer_space);
    strncpy((char *) buffer_ptr, s, buffer_space);
    if(len>buffer_space){
        buffer_ptr[buffer_space-1]=0;
        buffer_write_adv(&l->logbuf, buffer_space);
    }else{
        buffer_ptr[len]=0;
        buffer_write_adv(&l->logbuf, len+1);
    }
}

void log_sendf(struct log* l,const char *fmt, ...){
    char buffer[2048];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer,2048,fmt,args);
    log_send(l,buffer);
}



void
log_write(struct selector_key * key){
    struct log* l;
    l = GET_LOG(key);
    size_t buffer_size;
    uint8_t * buffer_ptr = buffer_read_ptr(&l->logbuf, &buffer_size);
    if(buffer_size>0){
        ssize_t written_bytes = write(l->writefd, buffer_ptr, buffer_size);
        buffer_read_adv(&l->logbuf, written_bytes);
//    }else{
//        selector_set_interest(l->selector, l->writefd, OP_NOOP);
    }

}

void log_close(struct selector_key * key){
    struct log* l;
    l = GET_LOG(key);
    close(l->writefd); //free malloc
    closelog();
}

