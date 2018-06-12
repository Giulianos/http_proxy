#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <response_parser/response_parser.h>
#include "response_parser_private.h"

struct response_parser {
    /** State */
    response_parser_state_t state;
    response_parser_state_t next_state;

    /** I/O Buffers */
    buffer * in_buffer;
    buffer * out_buffer;
    buffer * trans_buffer;

    /** Code */
    char code[4];
    size_t code_index;

    /** Text */
    char * text;
    size_t text_index;

    /** Header name */
    char * header_name;
    size_t header_name_index;

    /** Header value */
    char * header_value;
    size_t header_value_index;

    /** Chunk Size */
    char * chunk_size_str;
    size_t chunk_size_str_index;
    size_t chunk_size;

    /** Content length */
    ssize_t content_length;

    /** Flags */
    bool response_has_body;
    bool lambda_transition;
    bool should_transform;
    bool final_chunk;
    bool done;

    /** Errors */
    enum response_parser_error error;

    /** Callbacks */
    void(*response_ended_callback)(void *);

    /** Callback info */
    void * callbacks_info;
};

response_parser_t
response_parser_new(response_parser_config_t config)
{
  response_parser_t parser = malloc(sizeof(struct response_parser));
  if(parser == NULL) {
    return NULL;
  }

  parser->state = VERSION;
  parser->in_buffer = config->in_buffer;
  parser->out_buffer = config->out_buffer;
  parser->trans_buffer = config->trans_buffer;
  parser->code_index = 0;
  parser->text = NULL;
  parser->text_index = 0;
  parser->header_name = NULL;
  parser->header_name_index = 0;
  parser->header_value = NULL;
  parser->header_value_index = 0;
  parser->chunk_size_str = NULL;
  parser->chunk_size_str_index = 0;
  parser->chunk_size = 0;
  parser->content_length = -1;
  parser->response_has_body = false;
  parser->lambda_transition = false;
  parser->should_transform = false;
  parser->final_chunk = false;
  parser->done = false;
  parser->response_ended_callback = config->response_ended_callback;
  parser->error = RESPONSE_PARSER_NO_ERROR;
  parser->callbacks_info = config->callbacks_info;

  return parser;
}

int
response_parser_parse(response_parser_t parser)
{
  size_t parsed_bytes;
  size_t in_buffer_size;
  size_t out_buffer_space;
  size_t trans_buffer_space;

  size_t current = 0;

  uint8_t * in_buffer_ptr = buffer_read_ptr (parser->in_buffer, &in_buffer_size);
  uint8_t * out_buffer_ptr = buffer_write_ptr (parser->out_buffer, &out_buffer_space);
  //uint8_t * trans_buffer_ptr = buffer_write_ptr (parser->trans_buffer, &trans_buffer_space);

  if(!parser->should_transform) {
    parsed_bytes = in_buffer_size < out_buffer_space ? in_buffer_size : out_buffer_space;
  } else {
    parsed_bytes = in_buffer_size < trans_buffer_space ? in_buffer_size : trans_buffer_space;
  }

  while(current < parsed_bytes && !parser->done) {
    switch(parser->state) {
      case VERSION:
        dprintf(STDERR_FILENO, "[PARSER_STATE]VERSION\n");
        if(in_buffer_ptr[current] == ' ') {
          parser->next_state = CODE;
          parser->state = SPACES;
        } else if(!res_is_version_char (in_buffer_ptr[current])) {
          parser->done = true;
          parser->error = RESPONSE_PARSER_ERROR;
        }
        break;
      case CODE:
        dprintf(STDERR_FILENO, "[PARSER_STATE]CODE\n");
        if(parser->code_index == 3 && in_buffer_ptr[current] == ' ') {
          parser->code[4] = '\0';
          parser->response_has_body = res_check_response_has_body(parser->code);
          parser->next_state = TEXT;
          parser->state = SPACES;
        } else if(parser->code_index < 3 && !isdigit(in_buffer_ptr[current])) {
          parser->done = true;
          parser->error = RESPONSE_PARSER_ERROR;
        } else {
          parser->code[parser->code_index] = in_buffer_ptr[current];
          parser->code_index++;
        }
        break;
      case TEXT:
        dprintf(STDERR_FILENO, "[PARSER_STATE]TEXT\n");
        if(in_buffer_ptr[current] == '\n') {
          if(!res_check_temp_space (&parser->text, parser->text_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->text[parser->text_index] = '\0';
          parser->state = RES_HEADER_NAME;
        } else if(in_buffer_ptr[current] == '\r') {
          parser->next_state = TEXT;
          parser->state = CRS;
        } else if(!res_is_text_char(in_buffer_ptr[current])) {
          parser->done = true;
          parser->error = RESPONSE_PARSER_ERROR;
        } else if(res_is_text_char(in_buffer_ptr[current])){
          if(!res_check_temp_space (&parser->text, parser->text_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->text[parser->text_index] = in_buffer_ptr[current];
          parser->text_index++;
        }
        break;
      case RES_HEADER_NAME:
        dprintf(STDERR_FILENO, "[PARSER_STATE]RES_HEADER_NAME\n");
        if(res_is_header_name_char (in_buffer_ptr[current])) {
          if(!res_check_temp_space (&parser->header_name, parser->header_name_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->header_name[parser->header_name_index] = in_buffer_ptr[current];
          parser->header_name_index++;
        } else if(in_buffer_ptr[current] == ':') {
          if (!res_check_temp_space(&parser->header_name, parser->header_name_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->header_name[parser->header_name_index] = '\0';
          parser->next_state = RES_HEADER_VALUE;
          parser->state = SPACES;
        } else if(in_buffer_ptr[current] == '\n') {
          if(parser->content_length > 0 && parser->response_has_body) {
            parser->state = BODY;
          } else if(parser->content_length < 0){
            if (!res_check_temp_space(&parser->header_name, parser->header_name_index)) {
              parser->done = true;
              parser->error = RESPONSE_PARSER_ERROR;
            }
            parser->header_name[parser->header_name_index] = '\0';
            parser->state = CHUNK_SIZE;
          } else if(!parser->response_has_body || parser->content_length == 0) {
            /** Response ended! */
            if(parser->response_ended_callback == NULL) {
              parser->done = true;
              parser->error = RESPONSE_PARSER_ERROR;
            } else {
              parser->response_ended_callback(parser->callbacks_info);
              parser->done = true;
            }
          }
        } else if(in_buffer_ptr[current] == '\r') {
          parser->next_state = RES_HEADER_NAME;
          parser->state = CRS;
        } else {
          parser->done = true;
          parser->error = RESPONSE_PARSER_ERROR;
        }

        break;
      case RES_HEADER_VALUE:
        dprintf(STDERR_FILENO, "[PARSER_STATE]RES_HEADER_VALUE\n");
        if(res_is_header_value_char(in_buffer_ptr[current])) {
          if(!res_check_temp_space (&parser->header_value, parser->header_value_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->header_value[parser->header_value_index] = in_buffer_ptr[current];
          parser->header_value_index++;
        } else if(in_buffer_ptr[current] == '\n') {
          if(!res_check_temp_space (&parser->header_value, parser->header_value_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->header_value[parser->header_value_index] = '\0';

          if(parser->content_length == -1 &&
              (res_strncmp_case_insensitive(parser->header_name, "content-length", 15) == 0)) {
            parser->content_length = (ssize_t)atoi(parser->header_value);
            dprintf(STDERR_FILENO, "[PARSER_INFO]Content_Length:%d\n", parser->content_length);
          }

          parser->header_name_index = 0;
          parser->header_value_index = 0;
          parser->state = RES_HEADER_NAME;
        } else if(in_buffer_ptr[current] == '\r') {
          parser->next_state = RES_HEADER_VALUE;
          parser->state = CRS;
        } else {
          parser->done = true;
          parser->error = RESPONSE_PARSER_ERROR;
        }
        break;
      case BODY:
        dprintf(STDERR_FILENO, "[PARSER_STATE]BODY\n");
        dprintf(STDERR_FILENO, "[PARSER_INFO]Remaining length:%d\n", parser->content_length);
        parser->content_length--;
        if(parser->content_length == 0) {
          /** Response ended! */
          dprintf(STDERR_FILENO, "CALLBACK!\n");
          if(parser->response_ended_callback == NULL) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          } else {
            parser->response_ended_callback(parser->callbacks_info);
            parser->done = true;
          }
        }
        break;
      case CHUNK_SIZE:
        dprintf(STDERR_FILENO, "[PARSER_STATE]CHUNK_SIZE\n");
        if(isdigit(in_buffer_ptr[current])) {
          if(!res_check_temp_space (&parser->chunk_size_str, parser->chunk_size_str_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->chunk_size_str[parser->chunk_size_str_index] = in_buffer_ptr[current];
          parser->chunk_size_str_index++;
        } else {
          if(!res_check_temp_space (&parser->chunk_size_str, parser->chunk_size_str_index)) {
            parser->done = true;
            parser->error = RESPONSE_PARSER_ERROR;
          }
          parser->chunk_size_str[parser->chunk_size_str_index] = '\0';
          dprintf(STDERR_FILENO, "[PARSER_INFO]Chunk size: %s\n", parser->chunk_size_str);
          parser->chunk_size = (size_t)strtol(parser->chunk_size_str, NULL, 16);
          if(parser->chunk_size == 0) {
            parser->final_chunk = true;
            dprintf(STDERR_FILENO, "this is final chunk\n");
          }
          if(in_buffer_ptr[current] == '\n') {
            parser->state = CHUNK_BODY;
          } else if(in_buffer_ptr[current] == '\r') {
            parser->next_state = CHUNK_SIZE;
            parser->state = CRS;
          } else {
            parser->state = CHUNK_EXT;
          }
        }
        break;
      case CHUNK_EXT:
        dprintf(STDERR_FILENO, "[PARSER_STATE]CHUNK_EXT\n");
        if(in_buffer_ptr[current] == '\n') {
          parser->state = CHUNK_BODY;
        } else if(in_buffer_ptr[current] == '\r') {
          parser->next_state = CHUNK_EXT;
          parser->state = CRS;
        }
        break;
      case CHUNK_BODY:
        dprintf(STDERR_FILENO, "[PARSER_STATE]CHUNK_BODY\n");
        if(parser->chunk_size == 0 && in_buffer_ptr[current] == '\n') {
          if(parser->final_chunk) {
            if(parser->response_ended_callback == NULL) {
              parser->done = true;
              parser->error = RESPONSE_PARSER_ERROR;
            } else {
              parser->response_ended_callback(parser->callbacks_info);
              parser->done = true;
            }
            break;
          }
          /** do sth with chunk */
          parser->chunk_size_str_index = 0;
          parser->chunk_size = 0;
          parser->state = CHUNK_SIZE;
        } else if(in_buffer_ptr[current] == '\r') {
          dprintf(STDERR_FILENO, "[PARSER_INFO]Consuming CR, size= %d\n", parser->chunk_size);
          parser->next_state = CHUNK_BODY;
          parser->state = CRS;
        } else if(parser->chunk_size == 0) {
          parser->done = true;
          parser->error = RESPONSE_PARSER_ERROR;
        } else {
          parser->chunk_size--;
        }
        break;
      case SPACES:
        dprintf(STDERR_FILENO, "[PARSER_STATE]SPACES\n");
        if(in_buffer_ptr[current] != ' ') {
          parser->state = parser->next_state;
          parser->lambda_transition = true;
        }
        break;
      case CRS:
        dprintf(STDERR_FILENO, "[PARSER_STATE]CRS\n");
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

  if(parser->error != RESPONSE_PARSER_NO_ERROR) {
    dprintf(STDERR_FILENO, "estado de error: %d\n", parser->state);
    return -1;
  }

  return 0;

}