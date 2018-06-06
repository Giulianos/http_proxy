#include "actions.h"
#include <config/config.h>
#include <metric/metric.h>
#include <string.h>
#include <stdlib.h>
#include <protocol/protocol.h>
#include <stdio.h>

void
send_end_of_list(int msg_type);

void
send_error(unsigned char msg_type);

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
  send_end_of_list(LIST_METRICS);
}
void
send_list_configs()
{
  int i = 0;

  for(; i < config_get_size(); i++) {
    printf("called send config with %d\n", i);
    send_config(i);
  }
  send_end_of_list(LIST_CONFIGS);
}

void
send_end_of_list(int msg_type)
{
  msg_t * msg;

  msg = malloc(sizeof(msg_t));
  msg->type = msg_type;
  msg->buffer_size = 0;

  q_offer(msg);
}

void
send_metric(unsigned char metric)
{
  msg_t * msg;
  char * name = metric_get_name(metric);
  if(name == NULL) {
    send_error(GET_METRIC);
    return;
  }
  char * value = metric_get_from_index(metric);
  size_t name_len = strlen(name);
  size_t value_len = strlen(value);

  if(value == NULL) {
    send_error(GET_METRIC);
    return;
  }

  /** buffer = "(" + metric_num(unsigned int) + ")" + name + ": " + value + "\0" */
  msg = malloc(sizeof(msg_t));
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
  if(name == NULL){
    send_error(GET_CONFIG);
    return;
  }
  char * value = config_get_from_index(config);
  size_t name_len = strlen(name);
  size_t value_len = strlen(value);
/** TODO: error management (send error msg, config does not exist) */
  if(value == NULL) {
    send_error(GET_CONFIG);
    return;
  }

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
  if(value_len < 0) {
    send_error(SET_CONFIG);
    return;
  }

  char * val = malloc(value_len);
  memcpy(val, value, value_len);

  free(config_get_from_index(config));
  /** TODO: if config_set... returns <0 error management */
  config_set_from_index(config, val);
  printf("set %s\n", config_get_from_index(config));
}

void
send_error(unsigned char msg_type)
{
  msg_t * msg;

  msg = malloc(sizeof(msg_t));
  msg->type = ERROR;
  msg->buffer_size = 0;
  msg->param = msg_type;

  q_offer(msg);
}