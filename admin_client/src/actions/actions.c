#include <actions/actions.h>
#include <msg_queue/msg_queue.h>
#include <messages/messages.h>
#include <protocol/protocol.h>
#include <stdlib.h>
#include <string.h>

void
req_list_metrics()
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = LIST_METRICS;
  msg->bytes = sizeof(unsigned char *) * 2;

  offer(msg);
}

void
req_list_configs()
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = LIST_CONFIGS;
  msg->bytes = sizeof(unsigned char *) * 2;

  offer(msg);
}

void
req_get_metric(char * metric)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = GET_METRIC;
  msg->param = malloc(strlen(metric));
  strcpy(msg->param, metric);
  msg->bytes = sizeof(unsigned char *) * 3;

  offer(msg);
}

void
req_get_config(char * config)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = GET_METRIC;
  msg->param = malloc(strlen(config));
  strcpy(msg->param, config);
  msg->bytes = sizeof(unsigned char *) * 3;

  offer(msg);
}

void
req_set_config(char * config, char * value)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = GET_METRIC;
  msg->param = malloc(strlen(config));
  strcpy(msg->param, config);
  msg->buffer_size = strlen(value);
  strcpy(msg->buffer_size, value);
  msg->bytes = sizeof(unsigned char *) * (3 + msg->buffer_size) + sizeof(int);

  offer(msg);
}
