#ifndef HANDLERS_H
#define HANDLERS_H

#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sys/socket.h>
#include <selector/selector.h>

typedef struct addr_data * addr_data_t;

struct addr_data {
    struct sockaddr * addr;
    socklen_t len;
    struct sctp_sndrcvinfo sri;
    int msg_flags;
};

void
admin_read_handler(struct selector_key * key);
void
admin_write_handler(struct selector_key * key);

#endif
