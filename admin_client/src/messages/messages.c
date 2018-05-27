#include <messages/messages.h>
#include <serializer/serializer.h>
#include <stdlib.h>
#include <stdio.h>
#include <protocol/protocol.h>

ssize_t
send_msg(addr_data_t servdata, int socket, msg_t * msg)
{
  unsigned char * pointer;
  unsigned char buffer[MAX_MSG_SIZE+1];

  pointer = serialize_msg(buffer, msg);


  return sctp_sendmsg(socket, msg, pointer - buffer, servdata->addr, servdata->len, 0, 0,
                servdata->sri.sinfo_stream, 0, 0);
}

ssize_t
rcv_msg(addr_data_t servdata, int socket, msg_t * msg)
{
  ssize_t read_quan;
  unsigned char buffer[MAX_MSG_SIZE+1];
  struct sockaddr_in peer;
  socklen_t len = sizeof(peer);
  int msg_flags;

  read_quan = sctp_recvmsg(socket, buffer, sizeof(buffer), (struct sockaddr *)&peer,
      &len, &servdata->sri, &msg_flags);

  deserialize_msg(buffer, msg);

  return read_quan;
}

void
print_msg(msg_t * msg)
{
  int i;

  for(i = 0; i < msg->buffer_size; i++) {
    putchar(msg->buffer[i]);
  }
  putchar('\n');
}