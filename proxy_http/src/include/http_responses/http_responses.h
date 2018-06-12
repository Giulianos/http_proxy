#ifndef HTTPRESPONSES_H
#define HTTPRESPONSES_H

#include <stdlib.h>
#include <stdbool.h>
#include <buffer/buffer.h>

typedef enum http_response_builder_state {
  HTTP_RESPONSE_VERSION=0,
  HTTP_RESPONSE_STATUS_CODE,
  HTTP_RESPONSE_STATUS_TEXT,
  HTTP_RESPONSE_CONNECTION_HEADER,
  HTTP_RESPONSE_CONTENT_LENGTH_HEADER,
    HTTP_RESPONSE_CONTENT_LENGTH_HEADER_VALUE,
  HTTP_RESPONSE_BODY_PRETITLE,
  HTTP_RESPONSE_BODY_TITLE_CODE,
  HTTP_RESPONSE_BODY_TITLE_MSG,
  HTTP_RESPONSE_BODY_PREH1,
  HTTP_RESPONSE_BODY_H1_CODE,
  HTTP_RESPONSE_BODY_H1_MSG,
  HTTP_RESPONSE_BODY_END,
} http_response_state_t;

typedef struct http_response {
    /** Message info */
    unsigned int code;
    const char * msg;

    /** I/O */
    buffer * out_buffer;
    size_t written;

    /** Response builder state */
    http_response_state_t state;
    size_t section_index;

} http_response_t;

/**
 * Writes response messages on demand.
 *
 * Note: Unexpected results may occur if response is modified after first call.
 *
 * @param response response info
 * @return Whether the message was completly written to buffer or not
 */
bool
http_responses_write(http_response_t * response);


#endif
