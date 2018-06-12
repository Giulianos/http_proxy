#include <messages/messages.h>
#include <serializer/serializer.h>
#include <stdlib.h>
#include <stdio.h>
#include <protocol/protocol.h>
#include <errno.h>
#include <string.h>
#include <netinet/sctp.h>


char *
error_msg_to_string(enum error_type type);

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

  if(msg->type == ERROR) {
    printf("Error: %s\n", error_msg_to_string((enum error_type)msg->param));
    return;
  }

  if(msg->type == SET_CONFIG && msg->buffer_size == 0) {
    printf("Seteo realizado!\n");
    return;
  }

  for(i = 0; i < msg->buffer_size; i++) {
    putchar(msg->buffer[i]);
  }
  putchar('\n');
}

char *
error_msg_to_string(enum error_type type)
{
  switch(type) {
    case CONFIG_NOT_FOUND:
      return "Configuracion no encontrada.";
    case METRIC_NOT_FOUND:
      return "Metrica no encontrada.";
    case CONFIG_NOT_SET:
      return "No pudo setearse la configuracion.";
    case INVALID_LENGTH:
      return "Longitud invalida.";
    case UNEXPECTED_ERROR:
      return "Error inesperado. Intentelo denuevo.";
    default:
      return "Error desconocido.";
  }
}