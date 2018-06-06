#ifndef HANDLERS_H
#define HANDLERS_H

#include <selector/selector.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sys/socket.h>

typedef struct addr_data * addr_data_t;

struct addr_data {
    struct sockaddr * addr;
    socklen_t addr_len;
    struct sctp_sndrcvinfo * sri;
    struct sockaddr_in * peer;
    socklen_t peer_len;
};

void
admin_read_handler(struct selector_key * key);
void
admin_write_handler(struct selector_key * key);
void
stdin_read_handler(struct selector_key * key);
void
stdout_write_handler(struct selector_key * key);

#endif
