#ifndef CLIENT_REMOTE_HANDLERS_H
#define CLIENT_REMOTE_HANDLERS_H

#include <selector/selector.h>

void remote_read(struct selector_key* key);

void remote_write(struct selector_key* key);

void remote_block(struct selector_key* key);

void remote_close(struct selector_key* key);

#endif
