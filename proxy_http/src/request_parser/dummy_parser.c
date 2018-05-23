#include <request_parser/request_parser.h>
#include <stdlib.h>
#include <ctype.h>
#include <printf.h>
#include <memory.h>

#define REQUEST_PARSER_BUFFER_SIZE 100
#define HEADER_NAME_PREALLOC 20


enum request_parser_state {
    METHOD,
    TARGET,
    VERSION,
    HEADER,
    HEADER_VALUE,
    BODY,
};

typedef enum request_parser_state request_parser_state_t;

struct request_parser {
  int in_fd;
  request_parser_state_t state;
  buffer * out_buffer;

  /** Callbacks */
  void * data;
  void(*host_found_callback)(const char *, int, void*);

  /** Buffer for the header name */
  uint8_t * current_header_name;
  uint8_t * current_header_value;
  size_t header_index;
  bool * ready;
};



request_parser_t
request_parser_new(request_parser_config_t config)
{
  request_parser_t parser = malloc(sizeof(struct request_parser));
  if(parser == NULL)
    return NULL;

  parser->in_fd = config->in_fd;
  parser->out_buffer = config->out_buffer;
  parser->host_found_callback = config->host_found_callback;
  parser->current_header_name = NULL;
  parser->current_header_value = NULL;
  parser->header_index = 0;
  parser->state = METHOD;
  parser->ready = config->ready_flag;
  *(parser->ready) = false;
  return parser;
}

int
request_parser_destroy(request_parser_t parser)
{
  if(parser->current_header_value != NULL) {
    free(parser->current_header_value);
  }
  if(parser->current_header_name != NULL) {
    free(parser->current_header_name);
  }
  free(parser);
  /** TODO: Check errors */
  return 0;
}

int
request_parser_parse(request_parser_t parser)
{
  /** Buffer used in the parsing */
  uint8_t temp_buffer[REQUEST_PARSER_BUFFER_SIZE];

  /** Check how many bytes to parse with temp_buffer and
   *  out_buffer size restrictions.
   */
  size_t space_in_buffer;
  uint8_t  * buffer_ptr = buffer_write_ptr(parser->out_buffer, &space_in_buffer);
  space_in_buffer = space_in_buffer > REQUEST_PARSER_BUFFER_SIZE ? REQUEST_PARSER_BUFFER_SIZE : space_in_buffer;

  /** Read the bytes from the file descriptor into the temp_buffer */
  ssize_t read_bytes = read(parser->in_fd, temp_buffer, space_in_buffer);

  memcpy(buffer_ptr, temp_buffer, read_bytes);

  size_t current = 0;

  while(current <= read_bytes) {
    switch (parser->state) {
      case METHOD:
        if(isalpha(temp_buffer[current])) {
          /** For now, just consume the bytes */
          current++;
          buffer_write_adv(parser->out_buffer, 1);
        } else if(temp_buffer[current] == ' ') {
          current++;
          buffer_write_adv(parser->out_buffer, 1);
          parser->state = TARGET;
        }
        break;
      case TARGET:
        /** For now, just consume every byte */
        if(temp_buffer[current] == ' ') {
          parser->state = VERSION;
        }
        current++;
        buffer_write_adv(parser->out_buffer, 1);
        break;
      case VERSION:
        if(temp_buffer[current] == '\n' || temp_buffer[current] == '\r') {
          if(temp_buffer[current+1] == '\n') {
            current+=2;
            buffer_write_adv(parser->out_buffer, 2);
          } else {
            current++;
            buffer_write_adv(parser->out_buffer, 1);
          }
          parser->state = HEADER;
        } else {
          current++;
          buffer_write_adv(parser->out_buffer, 1);
        }
        break;
      case HEADER:
        if(temp_buffer[current] == '\n') {
          current++;
          buffer_write_adv(parser->out_buffer, 1);
          // parser->state = BODY;
          *(parser->ready) = true;
          return 0;
        }
        if(temp_buffer[current] == ':') {
          if(parser->header_index%HEADER_NAME_PREALLOC == 0) {
            parser->current_header_name = realloc(parser->current_header_name, HEADER_NAME_PREALLOC+1);
          }
          parser->current_header_name[parser->header_index] = '\0';
          parser->header_index = 0;
          parser->state = HEADER_VALUE;
          current++;
          buffer_write_adv(parser->out_buffer, 1);
        } else {
          if(parser->header_index%HEADER_NAME_PREALLOC == 0) {
            parser->current_header_name = realloc(parser->current_header_name, HEADER_NAME_PREALLOC+1);
          }
          parser->current_header_name[parser->header_index] = temp_buffer[current];
          parser->header_index++;
          current++;
          buffer_write_adv(parser->out_buffer, 1);
        }
        break;
      case HEADER_VALUE:
        if(temp_buffer[current] == '\n') {
          current++;
          buffer_write_adv(parser->out_buffer, 1);
          if(parser->header_index%HEADER_NAME_PREALLOC == 0) {
            parser->current_header_value = realloc(parser->current_header_value, HEADER_NAME_PREALLOC+1);
          }
          parser->current_header_value[parser->header_index] = '\0';
          parser->header_index = 0;
          /** Check if read header is Host */
          if(strcmp(parser->current_header_name, "Host") == 0) {
            if(parser->host_found_callback != NULL) {
              /** TODO: parse port */
              parser->host_found_callback(parser->current_header_value, 80, parser->data);
            }
          }
          free(parser->current_header_name);
          free(parser->current_header_value);
          parser->current_header_name = NULL;
          parser->current_header_value = NULL;
          parser->state = HEADER;
        } else {
          if(parser->header_index%HEADER_NAME_PREALLOC == 0) {
            parser->current_header_value = realloc(parser->current_header_value, HEADER_NAME_PREALLOC+1);
          }
          parser->current_header_value[parser->header_index] = temp_buffer[current];
          parser->header_index++;
          current++;
          buffer_write_adv(parser->out_buffer, 1);
        }
        break;
      case BODY:break;
    }
  }

  return 0;
}