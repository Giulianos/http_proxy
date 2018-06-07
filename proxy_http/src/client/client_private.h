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

#define GET_CLIENT(key) (client_t)((key)->data)

enum client_state {
  /** NEW STATES */
  NO_ORIGIN,
  SEND_REQ,
  READ_RESP,
  ERROR,
};

typedef enum client_state client_state_t;

struct host_details {
    unsigned int port;
    char fqdn[MAX_DOMAIN_NAME_LENGTH+1];
    struct addrinfo * resolved;
};

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

    /** Client's FDs and selector */
    int client_fd;
    int origin_fd;
    fd_selector selector;

    /** Request parser */
    requestState request_parser_state;

    //requestdata
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

    /** Request info */
    struct host_details host;
};


void
client_restart_state(client_t client);

void
client_free_resources(client_t client);

void
client_terminate(client_t client);

void
client_set_host(const char * host, int port, void * data);

static void *
request_resolv_blocking(void *data);

#endif