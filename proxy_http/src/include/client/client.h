#ifndef CLIENT_H
#define CLIENT_H

#include <selector/selector.h>

enum client_error {
    NO_ERROR = 0,
    KEEPALIVE_HOST_NO_MATCH,
    INVALID_HOST,
};

struct client_config {
    fd_selector selector;
    int fd;
};

typedef enum client_error client_error_t;
typedef struct client_cdt * client_t;

/** Creates a new client */
client_t
client_new(const struct client_config * config);

/** Handlers for client's events */
void
client_read(const struct selector_key * key);

void
client_write(const struct selector_key * key);

void
client_block(const struct selector_key * key);

void
client_close(const struct selector_key * key);


#endif
