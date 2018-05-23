#ifndef PROXY_HTTP_MANAGEMENT_H
#define PROXY_HTTP_MANAGEMENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#include <netdb.h>
#include <unistd.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <strings.h>
#include "buffer.h"
#include "../selector/selector.h"


#define ATTACHMENT(key) ( (struct management *)(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))
#define BUFFER_SIZE 1024

struct management{
    int                           client_fd;
    struct sockaddr_in6           addr_buf;

    buffer                        buffer_write, buffer_read;
    uint8_t                       raw_buffer_write[BUFFER_SIZE],
                                  raw_buffer_read[BUFFER_SIZE];

    uint8_t                       message_type;
    uint8_t                       cookie;
    uint16_t                      data_length;

};

void management_read(struct selector_key *key);
void management_write(struct selector_key *key);
void management_close(struct selector_key *key);
void management_destroy(struct management * m);
struct management* management_new(const int client_fd);
int create_management_socket(in_addr_t address,in_port_t port);

#endif //PROXY_HTTP_MANAGEMENT_H
