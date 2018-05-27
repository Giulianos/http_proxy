#include <print_queue/print_queue.h>
#include <stdlib.h>
#include <string.h>
#include <protocol/protocol.h>

static pqnode_t first = NULL;
static pqnode_t last = NULL;

msg_t *
pq_poll()
{
  pqnode_t aux;

  if(pq_is_empty()) {
    return NULL;
  }
  msg_t * msg = first->msg;
  aux = first;

  if(first == last)
    last = NULL;
  first = first->next;

  free(aux);

  return msg;
}

void
pq_offer(msg_t * msg)
{
  pqnode_t pqnode = malloc(sizeof(pqnode_t));
  pqnode->msg = msg;
  pqnode->next = NULL;
  if(pq_is_empty()) {
    first = pqnode;
    last = pqnode;
    return;
  }
  last->next = pqnode;
  last = pqnode;
}

int
pq_is_empty()
{
  return first == NULL;
}
