#include <print_queue/print_queue.h>
#include <stdlib.h>
#include <string.h>

static pqnode_t first = NULL;
static pqnode_t last = NULL;

unsigned char *
pq_poll()
{
  if(pq_is_empty()) {
    return NULL;
  }
  unsigned char * ret = first->str;

  if(first == last)
    last = NULL;
  first = first->next;

  return ret;
}


void
pq_offer(unsigned char * str)
{
  pqnode_t pqnode = malloc(sizeof(pqnode_t));
  pqnode->str = malloc(strlen(str));
  strcpy(pqnode->str, str);
  pqnode->next = NULL;
  if(pq_is_empty()) {
    first = pqnode;
    last = pqnode;
  }
  last->next = pqnode;
  last = pqnode;
}

int
pq_is_empty()
{
  return first == NULL;
}
