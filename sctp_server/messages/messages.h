#ifndef MESSAGES_H
#define MESSAGES_H


#include <protocol/protocol.h>
#include <handlers/handlers.h>
#include <unistd.h>

/**
 *
 * @param socket the socket in which is going to write the msg
 * @param msg the msg is going to write
 * @param size msg size
 * @return the quantity of bytes written.
 */

ssize_t
send_msg(addr_data_t servdata, int socket, msg_t * msg);

/**
 *
 * @param socket the socket from which is going to read the msg
 * @param msg a msg_t in which is going to save the msg read
 * @return the quantity of bytes read.
 */

ssize_t
rcv_msg(addr_data_t servdata, int socket, msg_t * msg);

void
print_msg(msg_t * msg);

#endif
