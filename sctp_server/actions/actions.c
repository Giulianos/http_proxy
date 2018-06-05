#include "actions.h"
#include <config/config.h>
#include <metric/metric.h>
#include <string.h>
#include <stdlib.h>
#include <protocol/protocol.h>
#include <stdio.h>

void
check_credentials(unsigned char * pass, int pass_len)
{

}
void
send_list_metrics()
{
  int i = 0;

  for(; i < metric_get_size(); i++) {
    send_metric(i);
  }
}
void
send_list_configs()
{
  int i = 0;

  for(; i < config_get_size(); i++) {
    printf("called send config with %d\n", i);
    send_config(i);
  }
}
void
send_metric(unsigned char metric)
{
  msg_t * msg = malloc(sizeof(msg_t));
  char * name = metric_get_name(metric);
  char * value = metric_get_from_index(metric);
  size_t name_len = strlen(name);
  size_t value_len = strlen(value);
/** TODO: error management (send error msg, metric does not exist) */
  if(name == NULL || value == NULL)
    return;

  /** buffer = "(" + metric_num(unsigned int) + ")" + name + ": " + value + "\0" */
  msg->buffer = malloc(name_len + value_len + 15);
  msg->type = GET_METRIC;
  msg->buffer_size = sprintf(msg->buffer, "(%d)%s: %s", metric, name, value);
  msg->buffer_size++;

  q_offer(msg);
}
void
send_config(unsigned char config)
{
  msg_t * msg;
  char * name = config_get_name(config);
  if(name == NULL)
    return;
  char * value = config_get_from_index(config);
  size_t name_len = strlen(name);
  size_t value_len = strlen(value);
/** TODO: error management (send error msg, config does not exist) */
  if(value == NULL)
    return;

  /** buffer = "(" + config_num(unsigned int) + ")" + name + ": " + value + "\0" */
  msg = malloc(sizeof(msg_t));
  msg->buffer = malloc(name_len + value_len + 15);
  msg->type = GET_CONFIG;
  msg->buffer_size = sprintf(msg->buffer, "(%d)%s: %s", config, name, value);
  msg->buffer_size++;

  q_offer(msg);
}
void
check_set_config(unsigned char config, unsigned char * value, int value_len)
{
  char * val = malloc(value_len);
  memcpy(val, value, value_len);

  free(config_get_from_index(config));
  /** TODO: if config_set... returns <0 error management */
  config_set_from_index(config, val);
  printf("set %s\n", config_get_from_index(config));
}