#ifndef PRINTQUEUE_H
#define PRINTQUEUE_H

typedef struct pqnode * pqnode_t;

struct pqnode{
  unsigned char * str;
  pqnode_t next;
};

unsigned char *
pq_poll();

void
pq_offer(unsigned char * str);

int
pq_is_empty();

#endif
