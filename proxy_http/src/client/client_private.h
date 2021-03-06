#ifndef CLIENT_PRIVATE_H
#define CLIENT_PRIVATE_H

#include <buffer/buffer.h>
#include <client/client.h>
#include <limits/limits.h>
#include <metric/metric.h>
#include <requestParser/requestParser.h>
#include <request_parser/request_parser.h>
#include <responseParser/responseParser.h>
#include <response_parser/response_parser.h>
#include <selector/selector.h>

#define GET_CLIENT(key) (client_t)((key)->data)

enum client_state
{
  /** NEW STATES */
  NO_ORIGIN,
  SEND_REQ,
  READ_RESP,
  CLI_ERROR,
};

typedef enum client_state client_state_t;

struct host_details
{
  unsigned int port;
  char fqdn[MAX_DOMAIN_NAME_LENGTH + 1];
  struct addrinfo* resolved;
};

struct client_cdt
{
  /** Client state */
  client_state_t state;
  client_error_t err;

  /** I/O Buffers */
  uint8_t* pre_req_parse_buf_mem;
  buffer pre_req_parse_buf;
  uint8_t* post_req_parse_buf_mem;
  buffer post_req_parse_buf;
  uint8_t* pre_res_parse_buf_mem;
  buffer pre_res_parse_buf;
  uint8_t* post_res_parse_buf_mem;
  buffer post_res_parse_buf;
  uint8_t* pre_transf_buf_mem;
  buffer pre_transf_buf;

  /** Client's FDs and selector */
  int client_fd;
  int origin_fd;
  int transf_in_fd;
  int transf_out_fd;
  fd_selector selector;

  /** Request parser */
  requestState request_parser_state;

  // requestdata
  RequestData req_data;
  ResponseData res_data;

  struct log* log;
#ifdef DUMMY_PARSERS
  request_parser_t request_parser;
#endif

  /** Response parser */
  response_parser_t response_parser;

  /** Request/response parsing state */
  bool request_complete;
  bool response_complete;

  /** Other flags */
  bool shouldTransform;

  /** Request info */
  struct host_details host;

  /** Connection time saving */
  connection_time_t connection_time;
};

void client_restart_state(client_t client);

void client_free_resources(client_t client);

void client_terminate(client_t client);

void client_set_host(const char* host, int port, void* data);

ssize_t dump_chunk_from_fd(int src_fd, buffer* dst_buffer);

ssize_t write_empty_chunk(buffer* dst_buffer);

#endif
