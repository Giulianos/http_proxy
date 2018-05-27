#include <msg_queue/msg_queue.h>
#include <protocol/protocol.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <serializer/serializer.h>

typedef struct qnode * qnode_t;

struct qnode {
    msg_t * msg;
    qnode_t next;
} ;

static qnode_t first = NULL;
static qnode_t last = NULL;

msg_t *
poll()
{
  msg_t * msg;

  if(q_is_empty()){
    return NULL;
  }

  msg = first->msg;
  if(first == last)
    last = NULL;
  first = first->next;

  return msg;
}


int
offer(msg_t * msg)
{
  qnode_t qnode = malloc(sizeof(qnode));
  if(qnode == NULL) {
    return -1;
  }
  qnode->msg = msg;
  qnode->next = NULL;

  if(q_is_empty()) {
    first = qnode;
    last = qnode;
    return 0;
  }

  last->next = qnode;
  return 0;
}


int
q_is_empty()
{
  return first == NULL;
}

