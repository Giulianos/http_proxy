#ifndef MESSAGES_H
#define MESSAGES_H

#include <protocol/protocol.h>

/**
 *
 * @param socket the socket in which is going to write the msg
 * @param msg the msg is going to write
 * @return the quantity of bytes written.
 */

int
send_msg(int socket, msg_t * msg);

/**
 *
 * @param socket the socket from which is going to read the msg
 * @param msg a msg_t in which is going to save the msg read
 * @return the quantity of bytes read.
 */

int
rcv_msg(int socket, msg_t * msg);

#endif
