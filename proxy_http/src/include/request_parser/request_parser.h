#ifndef REQUESTPARSER_H
#define  REQUESTPARSER_H

#include <buffer/buffer.h>

struct request_parser_config {
    buffer * in_buffer;
    buffer * out_buffer;
    bool * ready_flag;

    /** Callback called when the host is found, data is passed */
    void * data;
    void(*host_found_callback)(const char *, int, void*);
};

typedef struct request_parser * request_parser_t;
typedef struct request_parser_config * request_parser_config_t;

/**
 * Creates a new parser with the provided configuration
 *
 * @param config The configuration for the parser
 * @return The created buffer, NULL in case of error.
 */
request_parser_t
request_parser_new(request_parser_config_t config);

/**
 * Parses the characters from the fd provided in the configuration
 * and writes the consumed chars to the provided buffer.
 *
 * @param parser The parser to use
 * @return 0 if OK, < 0 in case of error.
 */
int
request_parser_parse(request_parser_t parser);

#endif
