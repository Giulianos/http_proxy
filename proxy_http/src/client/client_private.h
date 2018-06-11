#ifndef CLIENT_PRIVATE_H
#define CLIENT_PRIVATE_H

#include <buffer/buffer.h>
#include <limits/limits.h>
#include <request_parser/request_parser.h>
#include <response_parser/response_parser.h>
#include <requestParser/requestParser.h>
#include <selector/selector.h>
#include <client/client.h>
#include <responseParser/responseParser.h>
#include <metric/metric.h>

#define GET_CLIENT(key) (client_t)((key)->data)

enum client_state {
  /** NEW STATES */
  NO_ORIGIN,
  SEND_REQ,
  READ_RESP,
  CLI_ERROR,
};

typedef enum client_state client_state_t;

struct client_cdt {
    /** Client state */
    client_state_t state;
    client_error_t err;

    /** I/O Buffers */
    uint8_t  * pre_req_parse_buf_mem;
    buffer pre_req_parse_buf;
    uint8_t  * post_req_parse_buf_mem;
    buffer post_req_parse_buf;
    uint8_t  * pre_res_parse_buf_mem;
    buffer pre_res_parse_buf;
    uint8_t  * post_res_parse_buf_mem;
    buffer post_res_parse_buf;
    uint8_t  * pre_transf_buf_mem;
    buffer pre_transf_buf;

    /** Client's FDs and selector */
    int client_fd;
    int origin_fd;
    int transf_in_fd;
    int transf_out_fd;
    fd_selector selector;

    struct log* log;

    /** Request parser */
    request_parser_t request_parser;

    /** Response parser */
    ResponseData res_data;

    /** Request/response parsing state */
    bool request_complete;
    bool response_complete;

    /** Other flags */
    bool shouldTransform;

    /** Request info */
    char host[MAX_DOMAIN_NAME_LENGTH+1];
    unsigned int port;
    struct addrinfo * resolved;

    /** Connection time saving */
    connection_time_t connection_time;
};


void
client_restart_state(client_t client);

void
client_free_resources(client_t client);

void
client_terminate(client_t client);

void
client_set_host(host_details_t host, void* data);

ssize_t
dump_chunk_from_fd(int src_fd, buffer * dst_buffer);

ssize_t
write_empty_chunk(buffer * dst_buffer);

void
request_ended(void* data);

#endif