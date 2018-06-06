#ifndef PROTOCOL_H
#define PROTOCOL_H

#define ADMIN_PORT    9090
#define MAX_READ      1024
enum type{
    SEND_CRED = 0,
    LIST_METRICS,
    LIST_CONFIGS,
    GET_METRIC,
    GET_CONFIG,
    SET_CONFIG,
    ERROR
};

/** error types */
#define NO_METRIC     0
#define NO_CONFIG     1

#define MAX_MSG_SIZE 1024

typedef struct {
    unsigned char   type;
    unsigned char   param;
    int             buffer_size;
    unsigned char * buffer;
} msg_t;

#endif