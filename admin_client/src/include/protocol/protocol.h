#ifndef PROTOCOL_H
#define PROTOCOL_H

#define ADMIN_PORT    9090
#define MAX_READ      1024
#define SEND_CRED     0
#define LIST_METRICS  1
#define LIST_CONFIGS  2
#define GET_METRIC    3
#define GET_CONFIG    4
#define SET_CONFIG    5

/** error types */
#define NO_METRIC     0
#define NO_CONFIG     1

#define MAX_MSG_SIZE 1024

typedef struct {
    unsigned char   type;
    unsigned char   param;
    unsigned int    buffer_size;
    unsigned char * buffer;
} msg_t;

#endif