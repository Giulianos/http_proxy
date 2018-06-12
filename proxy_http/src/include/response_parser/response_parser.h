#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <buffer/buffer.h>

typedef struct response_parser * response_parser_t;

enum response_parser_error {
    RESPONSE_PARSER_NO_ERROR,
    RESPONSE_PARSER_ERROR,
};


struct response_parser_config {

    /** I/O Buffers */
    buffer * in_buffer;
    buffer * out_buffer;
    buffer * trans_buffer;

    /** Callbacks */
    void(*response_ended_callback)(void *);

    /** Callback info */
    void * callbacks_info;
};

typedef struct response_parser_config * response_parser_config_t;

response_parser_t
response_parser_new(response_parser_config_t config);

int
response_parser_parse(response_parser_t parser);

#endif