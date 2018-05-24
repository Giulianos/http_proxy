#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <protocol/protocol.h>

  typedef struct qnode * qnode_t;

  struct qnode {
    unsigned char * msg;
    int index;
    qnode_t next;
  } ;

  /**
   * @return the next node that needs to be sent
   * if there is no node, returns NULL
   */
  qnode_t
  poll();

  /**
   * offers a new qnode that needs to be sent
   * @param msg to be send
   * first time the msg is put on the queue so
   * a new qnode needs to be created
   * @return 0 if ok, -1 if not
   */
  int
  offer(msg_t * msg);

  /**
   * pushes the node at the beginning of the queue so that
   * it is the next one returned. This function is used when
   * a msg was polled but not sent completely.
   * @param qnode still needs to be sent
   * @return 0 if ok, -1 if not
   */
  int
  push(qnode_t qnode);


  /**
   * @return 1 if msq_q is empty, 0  if not
   */
  int
  q_is_empty();



#endif
