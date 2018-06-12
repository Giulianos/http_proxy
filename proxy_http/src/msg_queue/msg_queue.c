#include <msg_queue/msg_queue.h>
#include <protocol/protocol.h>
#include <serializer/serializer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct qnode* qnode_t;

struct qnode
{
  msg_t* msg;
  qnode_t next;
};

static int msg_fd = 0;
static fd_selector msg_s = NULL;
static qnode_t first = NULL;
static qnode_t last = NULL;

void
q_init(int fd, fd_selector s)
{
  msg_fd = fd;
  msg_s = s;
}

msg_t*
q_poll()
{
  msg_t* msg;

  if (q_is_empty()) {
    return NULL;
  }

  msg = first->msg;
  if (first == last)
    last = NULL;
  first = first->next;

  return msg;
}

int
q_offer(msg_t* msg)
{
  qnode_t qnode = malloc(sizeof(struct qnode));
  if (qnode == NULL) {
    return -1;
  }
  qnode->msg = msg;
  qnode->next = NULL;

  if (q_is_empty()) {
    first = qnode;
    last = qnode;
    return 0;
  }

  last->next = qnode;
  last = last->next;

  return 0;
}

int
q_is_empty()
{
  return first == NULL;
}
