#include <buffer/buffer.h>
#include <ctype.h>
#include <memory.h>
#include <printf.h>
#include <request_parser/request_parser.h>
#include <stdlib.h>

#define REQUEST_PARSER_BUFFER_SIZE 8192
#define HEADER_NAME_PREALLOC 20

enum request_parser_state
{
  METHOD,
  TARGET,
  VERSION,
  HEADER,
  HEADER_VALUE,
  BODY,
  SPAC_HVALUE,
};

typedef enum request_parser_state request_parser_state_t;

struct request_parser
{
  request_parser_state_t state;

  /** Buffers */
  buffer* in_buffer;
  buffer* out_buffer;

  /** Callbacks */
  void* data;
  void (*host_found_callback)(const char*, int, void*);

  /** Buffer for the header name */
  uint8_t* current_header_name;
  uint8_t* current_header_value;
  size_t header_index;
  bool* ready;
};

static void request_parser_dump(uint8_t* out, uint8_t* in, size_t bytes);

request_parser_t
request_parser_new(request_parser_config_t config)
{
  request_parser_t parser = malloc(sizeof(struct request_parser));
  if (parser == NULL)
    return NULL;

  parser->in_buffer = config->in_buffer;
  parser->out_buffer = config->out_buffer;
  parser->host_found_callback = config->host_found_callback;
  parser->ready = config->ready_flag;
  parser->data = config->data;
  parser->current_header_name = NULL;
  parser->current_header_value = NULL;
  parser->header_index = 0;
  parser->state = METHOD;
  *(parser->ready) = false;
  return parser;
}

int
request_parser_destroy(request_parser_t parser)
{
  if (parser->current_header_value != NULL) {
    free(parser->current_header_value);
  }
  if (parser->current_header_name != NULL) {
    free(parser->current_header_name);
  }
  free(parser);
  /** TODO: Check errors */
  return 0;
}

int
request_parser_parse(request_parser_t parser)
{

  /** Check how many bytes to parse with in_buffer and
   *  out_buffer size restrictions.
   */
  size_t in_buffer_available;
  uint8_t* in_buffer_ptr =
    buffer_read_ptr(parser->in_buffer, &in_buffer_available);
  size_t out_buffer_space;
  uint8_t* out_buffer_ptr =
    buffer_write_ptr(parser->out_buffer, &out_buffer_space);

  printf("I have %d in pre_req and %d space in post_req\n", in_buffer_available,
         out_buffer_space);

  size_t bytes_to_parse = (in_buffer_available < out_buffer_space)
                            ? in_buffer_available
                            : out_buffer_space;

  size_t current = 0;

  printf("parsing request...\n");

  while (current <= bytes_to_parse) {
    switch (parser->state) {
      case METHOD:
        if (isalpha(in_buffer_ptr[current])) {
          /** For now, just consume the bytes */
          current++;
        } else if (in_buffer_ptr[current] == ' ') {
          current++;
          parser->state = TARGET;
        }
        break;
      case TARGET:
        /** For now, just consume every byte */
        if (in_buffer_ptr[current] == ' ') {
          parser->state = VERSION;
        }
        current++;
        break;
      case VERSION:
        if (in_buffer_ptr[current] == '\n' || in_buffer_ptr[current] == '\r') {
          if (in_buffer_ptr[current + 1] == '\n') {
            current += 2;
          } else {
            current++;
          }
          parser->state = HEADER;
        } else {
          current++;
        }
        break;
      case HEADER:
        if (in_buffer_ptr[current] == '\n') {
          current++;
          // parser->state = BODY;
          *(parser->ready) = true;
          printf("Request complete!!\n");
          /** Dump the parsed in_buffer bytes into the out_buffer */
          memcpy(out_buffer_ptr, in_buffer_ptr, bytes_to_parse);
          buffer_read_adv(parser->in_buffer, bytes_to_parse);
          buffer_write_adv(parser->out_buffer, bytes_to_parse);
          return 0;
        }
        if (in_buffer_ptr[current] == ':') {
          if (parser->header_index % HEADER_NAME_PREALLOC == 0) {
            parser->current_header_name = realloc(
              parser->current_header_name, HEADER_NAME_PREALLOC + current);
          }
          parser->current_header_name[parser->header_index] = '\0';
          parser->header_index = 0;
          parser->state = SPAC_HVALUE;
          current++;
        } else {
          if (parser->header_index % HEADER_NAME_PREALLOC == 0) {
            parser->current_header_name = realloc(
              parser->current_header_name, HEADER_NAME_PREALLOC + current);
          }
          parser->current_header_name[parser->header_index] =
            in_buffer_ptr[current];
          parser->header_index++;
          current++;
        }
        break;
      case SPAC_HVALUE:
        if (in_buffer_ptr[current] != ' ') {
          parser->state = HEADER_VALUE;
        } else {
          current++;
        }
      case HEADER_VALUE:
        if (in_buffer_ptr[current] == '\n' || in_buffer_ptr[current] == '\r') {
          if (in_buffer_ptr[current + 1] == '\n') {
            current += 2;
          } else {
            current++;
          }
          if (parser->header_index % HEADER_NAME_PREALLOC == 0) {
            parser->current_header_value = realloc(
              parser->current_header_value, HEADER_NAME_PREALLOC + current);
          }
          parser->current_header_value[parser->header_index] = '\0';
          parser->header_index = 0;
          printf("Read a header, <%s>:%s\n", parser->current_header_name,
                 parser->current_header_value);
          /** Check if read header is Host */
          if (strcmp(parser->current_header_name, "Host") == 0) {
            if (parser->host_found_callback != NULL) {
              /** TODO: parse port */
              parser->host_found_callback(parser->current_header_value, 80,
                                          parser->data);
            }
          }
          free(parser->current_header_name);
          free(parser->current_header_value);
          parser->current_header_name = NULL;
          parser->current_header_value = NULL;
          parser->state = HEADER;
        } else {
          if (parser->header_index % HEADER_NAME_PREALLOC == 0) {
            parser->current_header_value = realloc(
              parser->current_header_value, HEADER_NAME_PREALLOC + current);
          }
          parser->current_header_value[parser->header_index] =
            in_buffer_ptr[current];
          parser->header_index++;
          current++;
        }
        break;
      case BODY:
        break;
    }
  }

  /** Dump the parsed in_buffer bytes into the out_buffer */
  memcpy(out_buffer_ptr, out_buffer_ptr, bytes_to_parse);
  buffer_read_adv(parser->in_buffer, bytes_to_parse);
  buffer_write_adv(parser->out_buffer, bytes_to_parse);
  return 0;
}