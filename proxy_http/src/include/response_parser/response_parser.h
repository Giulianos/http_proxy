#ifndef RESPONSEPARSER_H
#define RESPONSEPARSER_H

#include <buffer/buffer.h>

struct response_parser_config
{
  buffer* in_buffer;
  buffer* out_buffer;
  bool* ready_flag;
};

typedef struct response_parser* response_parser_t;
typedef struct response_parser_config* response_parser_config_t;

/**
 * Creates a new parser with the provided configuration
 *
 * @param config The configuration for the parser
 * @return The created buffer, NULL in case of error.
 */
response_parser_t response_parser_new(response_parser_config_t config);

/**
 * Parses the characters from the fd provided in the configuration
 * and writes the consumed chars to the provided buffer.
 *
 * @param parser The parser to use
 * @return 0 if OK, < 0 in case of error.
 */
int response_parser_parse(response_parser_t parser);

#endif
