#include <msg_queue/msg_queue.h>
#include <protocol/protocol.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static qnode_t first = NULL;
static qnode_t last = NULL;

qnode_t
poll()
{
  qnode_t qnode = malloc(sizeof(qnode_t));
  if(q_is_empty()){
    return NULL;
  }

  qnode->msg = first->msg;
  qnode->index = first->index;
  qnode->next = first->next;
  if(first == last)
    last = NULL;
  first = first->next;

  return qnode;
}


int
offer(msg_t * msg)
{
  qnode_t qnode = malloc(sizeof(qnode_t));
  qnode->msg = malloc(msg->bytes);
  memcpy(qnode->msg, msg, msg->bytes);
  qnode->index = 0;
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
push(qnode_t qnode)
{
  if(q_is_empty()) {
    qnode->next = NULL;
    first = qnode;
    last = qnode;
    return 0;
  }
  qnode->next = first->next;
  first = qnode;
  return 0;
}


int
q_is_empty()
{
  return first == NULL;
}

