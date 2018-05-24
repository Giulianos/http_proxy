#include <messages/messages.h>
#include <serializer/serializer.h>
#include <protocol/protocol.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_MSG_SIZE 1024

int
send_msg(int socket, msg_t * msg)
{
  ssize_t written_quan;
  unsigned char * buffer = (unsigned char *)malloc(msg->bytes);
  unsigned char * pointer;

  pointer = serialize_msg(buffer, msg);

  written_quan = write(socket, buffer, pointer-buffer);
  free((void *) buffer);

  return written_quan;
}

int
rcv_msg(int socket, msg_t * msg)
{
  int read_quan;
  unsigned char buffer[MAX_MSG_SIZE+1];

  read_quan = read(socket, buffer, MAX_MSG_SIZE);

  deserialize_msg(buffer, msg);

  return read_quan;
}