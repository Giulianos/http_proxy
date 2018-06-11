#ifndef REQUEST_PARSER_REQUEST_PARSER_H
#define REQUEST_PARSER_REQUEST_PARSER_H

#include <buffer/buffer.h>

typedef struct request_parser * request_parser_t;

enum request_parser_error {
    REQUEST_PARSER_NO_ERROR,
    REQUEST_PARSER_ERROR,
};

enum host_type {
    FQDN,
    IPV4,
    IPV6,
    FQDN_AND_PORT,
    IPV4_AND_PORT,
    IPV6_AND_PORT,
};

struct host_details {
    enum host_type type;
    char * host;
    unsigned int port;
};

typedef struct host_details * host_details_t;

struct request_parser_config {

    /** I/O Buffers */
    buffer * in_buffer;
    buffer * out_buffer;

    /** Callbacks */
    void(*got_host_callback)(host_details_t, void *);
    void(*request_ended_callback)(void *);

    /** Callback info */
    void * callbacks_info;
};

typedef struct request_parser_config * request_parser_config_t;

request_parser_t
request_parser_new(request_parser_config_t config);

int
request_parser_parse(request_parser_t parser);

int
request_parser_destroy(request_parser_t parser);

#endif //REQUEST_PARSER_REQUEST_PARSER_H
