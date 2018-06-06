#include <messages/messages.h>
#include <serializer/serializer.h>
#include <stdlib.h>
#include <stdio.h>
#include <protocol/protocol.h>
#include <errno.h>
#include <string.h>

ssize_t
send_msg(addr_data_t servdata, int socket, msg_t * msg)
{
  unsigned char * pointer;
  unsigned char buffer[MAX_MSG_SIZE+1];
  ssize_t sent_bytes;
  fd_set select_set;

  FD_SET(socket, &select_set);

  pointer = serialize_msg(buffer, msg);
  select(socket+1, NULL, &select_set, NULL, NULL);
  printf("before sending:\n");
//  printf("socket: %d, buffer: %x, sent_len: %d, addr: %x, struct_lent: %x, sri.sinfo: %d\n", socket, buffer, pointer - buffer, servdata->addr,
//                            servdata->len, servdata->sri.sinfo_stream);
  sent_bytes = sctp_sendmsg(socket, buffer, pointer - buffer, servdata->addr, servdata->addr_len, 0, 0,
                servdata->sri->sinfo_stream, 0, 0);
  if(sent_bytes <= 0) {
    printf("%s\n",strerror(errno));
  }

  return sent_bytes;
}

ssize_t
rcv_msg(addr_data_t servdata, int socket, msg_t * msg)
{
  ssize_t read_quan;
  unsigned char buffer[MAX_READ];
  int msg_flags;
  fd_set select_set;

  FD_SET(socket, &select_set);
  select(socket+1, &select_set, NULL, NULL, NULL);


  read_quan = sctp_recvmsg(socket, buffer, MAX_READ, (struct sockaddr *)servdata->peer,
      &servdata->peer_len, servdata->sri, &msg_flags);
  if(read_quan < 0) {
    printf("%s\n",strerror(errno));
  }

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