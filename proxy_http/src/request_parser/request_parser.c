#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "request_parser_private.h"

struct request_parser {
  /** State */
  request_parser_state_t state;
  request_parser_state_t next_state;

  /** I/O Buffers */
  buffer * in_buffer;
  buffer * out_buffer;

  /** Method */
  char method[MAX_METHOD_LEN + 1];
  size_t method_index;

  /** Target */
  char * target;
  size_t target_index;

  /** Header name */
  char * header_name;
  size_t header_name_index;

  /** Header value */
  char * header_value;
  size_t header_value_index;

  /** Content length */
  size_t content_length;

  /** Flags */
  bool request_has_body;
  bool lambda_transition;
  bool host_found;
  bool done;

  /** Errors */
  enum request_parser_error error;

  /** Host details */
  struct host_details host;

  /** Callbacks */
  void(*got_host_callback)(host_details_t, void *);
  void(*request_ended_callback)(void *);

  /** Callback info */
  void * callbacks_info;
};

request_parser_t
request_parser_new(request_parser_config_t config)
{
  request_parser_t parser = malloc(sizeof(struct request_parser));
  if(parser == NULL) {
      return NULL;
  }

  parser->state = METHOD;
  parser->in_buffer = config->in_buffer;
  parser->out_buffer = config->out_buffer;
  parser->method_index = 0;
  parser->target = NULL;
  parser->target_index = 0;
  parser->header_name = NULL;
  parser->header_name_index = 0;
  parser->header_value = NULL;
  parser->header_value_index = 0;
  parser->content_length = 0;
  parser->request_has_body = false;
  parser->lambda_transition = false;
  parser->host_found = false;
  parser->done = false;
  parser->error = REQUEST_PARSER_NO_ERROR;
  parser->got_host_callback = config->got_host_callback;
  parser->request_ended_callback = config->request_ended_callback;
  parser->callbacks_info = config->callbacks_info;

  return parser;
}

int
request_parser_parse(request_parser_t parser)
{
  size_t parsed_bytes;
  size_t in_buffer_size;
  size_t out_buffer_space;

  size_t current = 0;

  uint8_t * in_buffer_ptr = buffer_read_ptr (parser->in_buffer, &in_buffer_size);
  uint8_t * out_buffer_ptr = buffer_write_ptr (parser->out_buffer, &out_buffer_space);
  
  parsed_bytes = in_buffer_size < out_buffer_space ? in_buffer_size : out_buffer_space;

  while(current < parsed_bytes && !parser->done) {
    switch(parser->state) {
      case METHOD:
        dprintf (STDERR_FILENO, "[PARSER_STATE]Method\n");
        if(isalpha(in_buffer_ptr[current])) {
          parser->method[parser->method_index] = in_buffer_ptr[current];
          parser->method_index++;
        } else if(in_buffer_ptr[current] == ' ') {
          parser->method[parser->method_index] = '\0';
          parser->request_has_body = check_request_has_body(parser->method);
          if(parser->request_has_body)
            dprintf (STDERR_FILENO, "[PARSER_INFO]Should have body\n");
          parser->next_state = URL;
          parser->state = SPACES;
        } else {
          parser->done = true;
          parser->error = REQUEST_PARSER_ERROR;
        }
        break;
      case URL:
        dprintf (STDERR_FILENO, "[PARSER_STATE]URL\n");
        if(is_url_char (in_buffer_ptr[current])) {
          if(!check_temp_space (&parser->target, parser->target_index)) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
          }

          parser->target[parser->target_index] = in_buffer_ptr[current];
          parser->target_index++;
        } else if(in_buffer_ptr[current] == ' ') {
          if(!check_temp_space (&parser->target, parser->target_index)) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
          }
          parser->target[parser->target_index] = '\0';
          if(extract_host_from_target (parser->target, &parser->host)) {
            /** Host found! */
            if(parser->got_host_callback == NULL) {
                parser->done = true;
                parser->error = REQUEST_PARSER_ERROR;
            } else {
              parser->host_found = true;
              parser->got_host_callback(&parser->host, parser->callbacks_info);
            }
          }
          parser->next_state = VERSION;
          parser->state = SPACES;
        } else {
            parser->done = true;
            parser->error = REQUEST_PARSER_ERROR;
        }
        break;
      case VERSION:
        dprintf (STDERR_FILENO, "[PARSER_STATE]Version\n");
        if(in_buffer_ptr[current] == '\n') {
          parser->state = HEADER_NAME;
        } else if(in_buffer_ptr[current] == '\r') {
            parser->next_state = VERSION;
            parser->state = CRS;
        } else if(!is_version_char (in_buffer_ptr[current])) {
            parser->done = true;
            parser->error = REQUEST_PARSER_ERROR;
        }
        break;
      case HEADER_NAME:
        dprintf (STDERR_FILENO, "[PARSER_STATE]Header name\n");
        if(is_header_name_char (in_buffer_ptr[current])) {
          if(!check_temp_space (&parser->header_name, parser->header_name_index)) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
          }

          parser->header_name[parser->header_name_index] = in_buffer_ptr[current];
          parser->header_name_index++;
        } else if(in_buffer_ptr[current] == ':') {
          if(!check_temp_space (&parser->header_name, parser->header_name_index)) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
          }
          parser->header_name[parser->header_name_index] = '\0';

          parser->next_state = HEADER_VALUE;
          parser->state = SPACES;
        } else if(in_buffer_ptr[current] == '\r') {
          parser->next_state = HEADER_NAME;
          parser->state = CRS;
        } else if(in_buffer_ptr[current] == '\n') {
          dprintf (STDERR_FILENO, "[PARSER_INFO]End or body\n");
          if(parser->content_length > 0 && parser->request_has_body) {
            parser->state = BODY;
          } else if(!parser->request_has_body) {
            /** Request ended! */
            if(parser->request_ended_callback == NULL) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
            } else {
              parser->request_ended_callback(parser->callbacks_info);
            }

            parser->done = true;
          }
        } else {
            parser->done = true;
            parser->error = REQUEST_PARSER_ERROR;
        }

        break;
      case HEADER_VALUE:
        dprintf (STDERR_FILENO, "[PARSER_STATE]Header value\n");
        if(is_header_value_char (in_buffer_ptr[current])) {
          if(!check_temp_space (&parser->header_value, parser->header_value_index)) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
          }

          parser->header_value[parser->header_value_index] = in_buffer_ptr[current];
          parser->header_value_index++;
        } else if(in_buffer_ptr[current] == '\r') {
          parser->next_state = HEADER_VALUE;
          parser->state = CRS;
        } else if(in_buffer_ptr[current] == '\n') {
          if(!check_temp_space (&parser->header_value, parser->header_value_index)) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
          }
          parser->header_value[parser->header_value_index] = '\0';

          /** If we haven't found the host, look in the headers */
          if(!parser->host_found && strncmp_case_insensitive (parser->header_name, "host", strlen("host")+1) == 0) {
            parser->host.host = parser->header_value;
            fill_host_details(&parser->host);

            if(parser->got_host_callback == NULL) {
                parser->done = true;
                parser->error = REQUEST_PARSER_ERROR;
            } else {
              parser->host_found = true;
              parser->got_host_callback(&parser->host, parser->callbacks_info);
            }

            /** Don't reuse temp space for next header */
            parser->header_value = NULL;
          }

          parser->header_name_index = 0;
          parser->header_value_index = 0;
          parser->state = HEADER_NAME;
        } else {
            parser->done = true;
            parser->error = REQUEST_PARSER_ERROR;
        }

        break;
      case BODY:
        dprintf (STDERR_FILENO, "[PARSER_STATE]Body\n");
        if(parser->content_length == 0) {
          /** Request ended! */
          if(parser->request_ended_callback == NULL) {
              parser->done = true;
              parser->error = REQUEST_PARSER_ERROR;
          } else {
            parser->request_ended_callback(parser->callbacks_info);
          }

          parser->done = true;
        }
        parser->content_length--;
        break;
      case SPACES:
        if(in_buffer_ptr[current] != ' ') {
          parser->state = parser->next_state;
          parser->lambda_transition = true;
        }
        break;
      case CRS:
        if(in_buffer_ptr[current] != '\r') {
          parser->state = parser->next_state;
          parser->lambda_transition = true;
        }
      break;
    }
    if(!parser->lambda_transition) {
      current++;
    } else {
      parser->lambda_transition = false;
    }
  }

  memcpy(out_buffer_ptr, in_buffer_ptr, parsed_bytes);
  buffer_read_adv (parser->in_buffer, parsed_bytes);
  buffer_write_adv (parser->out_buffer, parsed_bytes);

  if(parser->error != REQUEST_PARSER_NO_ERROR) {
    return -1;
  }

  return 0;
}

int
request_parser_destroy(request_parser_t parser)
{
  if(parser == NULL) {
    return -1;
  }

  if(parser->target != NULL) {
      free (parser->target);
  }
  if(parser->header_name != NULL) {
      free (parser->header_name);
  }
  if(parser->header_value != NULL) {
      free (parser->header_value);
  }

  free (parser);

  return 0;
}
