#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <protocol/protocol.h>
#include <selector/selector.h>

/**
 *
 * @param fd associated to the msg queue
 */
void q_init(int fd, fd_selector s);

/**
 * @return the msg that needs to be sent (msg of the first node)
 * if there is no node, returns NULL
 */
msg_t* q_poll();

/**
 * offers a new qnode that needs to be sent
 * @param msg to be send
 * first time the msg is put on the queue so
 * a new qnode needs to be created
 * @return 0 if ok, -1 if not
 */
int q_offer(msg_t* msg);

/**
 * @return 1 if msq_q is empty, 0  if not
 */
int q_is_empty();

#endif
