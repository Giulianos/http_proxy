#ifndef PROTOCOL_H
#define PROTOCOL_H

#define ADMIN_PORT    9090
#define MAX_READ      1024
#define LIST_METRICS  0
#define LIST_CONFIGS  1
#define GET_METRIC    2
#define GET_CONFIG    3
#define SET_CONFIG    4
#define CLOSE         5
#define MAX_TYPE      6

typedef struct {
    int bytes;
    unsigned char type;
    unsigned char * param;
    int buffer_size;
    unsigned char * buffer;
} msg_t;

#endif