#include <protocol/protocol.h>
#include <stdlib.h>
#include <messages/messages.h>
#include <msg_queue/msg_queue.h>
#include <serializer/serializer.h>
#include <actions/actions.h>
#include "handlers.h"

void
admin_read_handler(struct selector_key * key)
{
  int rd_sz;
  int i = 0;
  addr_data_t admin_data = (addr_data_t) key->data;
  msg_t * msg = malloc(sizeof(msg_t));
  unsigned char buffer[MAX_READ];

  rd_sz = sctp_recvmsg(key->fd, buffer, MAX_READ, admin_data->addr, &admin_data->len, &admin_data->sri, &admin_data->msg_flags);

  if(rd_sz <= 0)
    return;

  deserialize_msg(buffer, msg);
  switch(msg->type) {
    case SEND_CRED+'0':
      check_credentials(msg->buffer, msg->buffer_size);
      break;
    case LIST_METRICS+'0':
      send_list_metrics();
      break;
    case LIST_CONFIGS+'0':
      send_list_configs();
      break;
    case GET_METRIC+'0':
      send_metric(msg->param);
      break;
    case GET_CONFIG+'0':
      send_config(msg->param);
      break;
    case SET_CONFIG+'0':
      check_set_config(msg->param, msg->buffer, msg->buffer_size);
      break;
  }
}

void
admin_write_handler(struct selector_key * key) {
  msg_t * msg;
  addr_data_t servdata = (addr_data_t) key->data;

  if (q_is_empty())
    return;

  msg = q_poll();

  send_msg(servdata, key->fd, msg);
}