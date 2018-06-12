#ifndef PRINTQUEUE_H
#define PRINTQUEUE_H

#include <protocol/protocol.h>

typedef struct pqnode* pqnode_t;

struct pqnode
{
  msg_t* msg;
  pqnode_t next;
};

msg_t* pq_poll();

void pq_offer(msg_t* msg);

int pq_is_empty();

#endif
