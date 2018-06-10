#ifndef CLIENT_TRANSFORMATION_HANDLERS_H
#define CLIENT_TRANSFORMATION_HANDLERS_H

#include <selector/selector.h>

void
transf_read(struct selector_key * key);

void
transf_write(struct selector_key * key);

void
transf_block(struct selector_key * key);

void
transf_close(struct selector_key * key);

#endif
