#include <response_parser/response_parser.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

struct response_parser {

    /** Buffers */
    buffer * in_buffer;
    buffer * out_buffer;

};



response_parser_t
response_parser_new(response_parser_config_t config)
{
  response_parser_t parser = malloc(sizeof(struct response_parser));
  if(parser == NULL)
    return NULL;

  parser->in_buffer = config->in_buffer;
  parser->out_buffer = config->out_buffer;

  return parser;
}

int
response_parser_destroy(response_parser_t parser)
{
  free(parser);
  /** TODO: Check errors */
  return 0;
}

int
response_parser_parse(response_parser_t parser)
{

  /** Check how many bytes to parse with in_buffer and
   *  out_buffer size restrictions.
   */
  size_t in_buffer_available;
  uint8_t  * in_buffer_ptr = buffer_read_ptr(parser->in_buffer, &in_buffer_available);
  size_t out_buffer_space;
  uint8_t  * out_buffer_ptr = buffer_write_ptr(parser->out_buffer, &out_buffer_space);

  size_t bytes_to_parse = (in_buffer_available < out_buffer_space) ? in_buffer_available : out_buffer_space;

  //printf("parsing response...\n");

  /** Dump the "parsed" in_buffer bytes into the out_buffer */
  memcpy(out_buffer_ptr, in_buffer_ptr, bytes_to_parse);
  buffer_read_adv(parser->in_buffer, bytes_to_parse);
  buffer_write_adv(parser->out_buffer, bytes_to_parse);

  return 0;
}