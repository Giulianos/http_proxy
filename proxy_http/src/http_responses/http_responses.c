#include "http_responses_templates.h"
#include <http_responses/http_responses.h>
#include <stdio.h>
#include <string.h>

static bool copy_str_to_buffer(uint8_t* buffer, uint8_t* str, size_t* offset,
                               size_t max_length);
static bool copy_num_to_buffer(uint8_t* buffer, unsigned int i, size_t* offset,
                               size_t max_length);

bool
http_responses_write(http_response_t* response)
{
  size_t buffer_space;
  uint8_t* buffer_ptr = buffer_write_ptr(response->out_buffer, &buffer_space);
  size_t index = 0;
  size_t prev_offset = 0;
  bool section_ended;
  bool response_ended = false;
  size_t msg_len = strlen(response->msg);

  while (index < buffer_space && !response_ended) {
    switch (response->state) {
      case HTTP_RESPONSE_VERSION:
        prev_offset = response->section_index;
        section_ended = copy_str_to_buffer(
          buffer_ptr + index, (uint8_t*)http_response_version,
          &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
          response->section_index = 0;
          response->state = HTTP_RESPONSE_STATUS_CODE;
        }
        break;
      case HTTP_RESPONSE_STATUS_CODE:
        prev_offset = response->section_index;
        section_ended =
          copy_num_to_buffer(buffer_ptr + index, response->code,
                             &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_STATUS_TEXT;
        }
        break;
      case HTTP_RESPONSE_STATUS_TEXT:
        prev_offset = response->section_index;
        section_ended =
          copy_str_to_buffer(buffer_ptr + index, (uint8_t*)response->msg,
                             &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_CONNECTION_HEADER;
        }
        break;
      case HTTP_RESPONSE_CONNECTION_HEADER:
        prev_offset = response->section_index;
        section_ended = copy_str_to_buffer(
          buffer_ptr + index, (uint8_t*)http_response_connection_header,
          &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_CONTENT_LENGTH_HEADER;
        }
        break;
      case HTTP_RESPONSE_CONTENT_LENGTH_HEADER:
        prev_offset = response->section_index;
        section_ended = copy_str_to_buffer(
          buffer_ptr + index, (uint8_t*)http_response_content_length_header,
          &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_CONTENT_LENGTH_HEADER_VALUE;
        }
        break;
      case HTTP_RESPONSE_CONTENT_LENGTH_HEADER_VALUE:
        prev_offset = response->section_index;
        section_ended =
          copy_num_to_buffer(buffer_ptr + index, (unsigned int)(msg_len*2 + 137),
                             &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_BODY_PRETITLE;
        }
        break;
      case HTTP_RESPONSE_BODY_PRETITLE:
        prev_offset = response->section_index;
        section_ended = copy_str_to_buffer(
          buffer_ptr + index, (uint8_t*)http_response_body_pretitle,
          &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_BODY_TITLE_CODE;
        }
        break;
      case HTTP_RESPONSE_BODY_TITLE_CODE:
        prev_offset = response->section_index;
        section_ended =
          copy_num_to_buffer(buffer_ptr + index, response->code,
                             &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_BODY_TITLE_MSG;
        }
        break;
      case HTTP_RESPONSE_BODY_TITLE_MSG:
        prev_offset = response->section_index;
        section_ended =
          copy_str_to_buffer(buffer_ptr + index, (uint8_t*)response->msg,
                             &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_BODY_PREH1;
        }
        break;
      case HTTP_RESPONSE_BODY_PREH1:
        prev_offset = response->section_index;
        section_ended = copy_str_to_buffer(
          buffer_ptr + index, (uint8_t*)http_response_body_preh1,
          &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_BODY_H1_CODE;
        }
        break;
      case HTTP_RESPONSE_BODY_H1_CODE:
        prev_offset = response->section_index;
        section_ended =
          copy_num_to_buffer(buffer_ptr + index, response->code,
                             &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_BODY_H1_MSG;
        }
        break;
      case HTTP_RESPONSE_BODY_H1_MSG:
        prev_offset = response->section_index;
        section_ended =
          copy_str_to_buffer(buffer_ptr + index, (uint8_t*)response->msg,
                             &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
            response->section_index = 0;
          response->state = HTTP_RESPONSE_BODY_END;
        }
        break;
      case HTTP_RESPONSE_BODY_END:
        prev_offset = response->section_index;
        section_ended = copy_str_to_buffer(
          buffer_ptr + index, (uint8_t*)http_response_body_end,
          &response->section_index, buffer_space - index);
        index += response->section_index - prev_offset;
        if (section_ended) {
          response_ended = true;
        }
        break;
    }
  }
  buffer_write_adv (response->out_buffer, index);
  return response_ended;
}

static bool
copy_str_to_buffer(uint8_t* buffer, uint8_t* str, size_t* offset,
                   size_t max_length)
{
  size_t index;

  for (index = 0; index < max_length && str[index + *offset] != '\0'; index++) {
    buffer[index] = str[index + *offset];
  }

  *offset += index;

  return str[*offset] == '\0';
}

static bool
copy_num_to_buffer(uint8_t* buffer, unsigned int i, size_t* offset,
                   size_t max_length)
{
  uint8_t temp_buffer[20];

  sprintf((char*)temp_buffer, "%d ", i);
  return copy_str_to_buffer(buffer, temp_buffer, offset, max_length);
}