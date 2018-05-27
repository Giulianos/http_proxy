#include <actions/actions.h>
#include <msg_queue/msg_queue.h>
#include <messages/messages.h>
#include <protocol/protocol.h>
#include <stdlib.h>
#include <string.h>

void
send_credentials(unsigned char * pass)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = SEND_CRED;
  msg->param = 0;
  msg->buffer_size = strlen(pass)+1;
  msg->buffer = malloc(msg->buffer_size);
  strncpy(msg->buffer, pass, msg->buffer_size);

  offer(msg);
}

void
req_list_metrics()
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = LIST_METRICS;
  msg->param = 0;
  msg->buffer_size = 0;

  offer(msg);
}

void
req_list_configs()
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = LIST_CONFIGS;
  msg->param = 0;
  msg->buffer_size = 0;

  offer(msg);
}

void
req_get_metric(unsigned char metric)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = GET_METRIC;
  msg->param = metric;
  msg->buffer_size = 0;

  offer(msg);
}

void
req_get_config(unsigned char config)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = GET_CONFIG;
  msg->param = config;
  msg->buffer_size = 0;

  offer(msg);
}

void
req_set_config(unsigned char config, unsigned char * value)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = SET_CONFIG;
  msg->param = config;
  msg->buffer_size = strlen(value)+1;
  msg->buffer = malloc(msg->buffer_size);
  strncpy(msg->buffer, value, msg->buffer_size);

  offer(msg);
}

void
error_handler(unsigned char error_type)
{

}