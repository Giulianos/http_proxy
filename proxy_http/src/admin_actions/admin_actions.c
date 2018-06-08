#include <admin_actions/admin_actions.h>
#include <config/config.h>
#include <metric/metric.h>
#include <string.h>
#include <stdlib.h>
#include <protocol/protocol.h>
#include <stdio.h>

void
send_end_of_list(unsigned char msg_type);

void
send_error(unsigned char msg_type);

void
check_credentials(unsigned char * pass, int pass_len)
{

}
void
send_list_metrics()
{
  unsigned char i = 0;

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
send_end_of_list(unsigned char msg_type)
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
  char * name;
  char * value;
  size_t name_len;
  size_t value_len;

  name = metric_get_name(metric);
  if(name == NULL) {
    send_error(METRIC_NOT_FOUND);
    return;
  }
  value = metric_get_from_index(metric);
  name_len = strlen(name);
  value_len = strlen(value);

  if(value == NULL) {
    send_error(UNEXPECTED_ERROR);
    return;
  }

  /** buffer = "(" + metric_num(unsigned int) + ")" + name + ": " + value + "\0" */
  msg = malloc(sizeof(msg_t));
  if(msg == NULL) {
    send_error(UNEXPECTED_ERROR);
    return;
  }
  msg->buffer = malloc(name_len + value_len + 15);
  if(msg->buffer == NULL) {
    send_error(UNEXPECTED_ERROR);
    free(msg);
    return;
  }
  msg->type = GET_METRIC;
  msg->buffer_size = sprintf((char *)msg->buffer, "(%d)%s: %s", metric, name, value);
  msg->buffer_size++;

  q_offer(msg);
}
void
send_config(unsigned char config)
{
  msg_t * msg;
  char * name;
  char * value;
  size_t name_len;
  size_t value_len;

  name = config_get_name(config);
  if(name == NULL){
    send_error(CONFIG_NOT_FOUND);
    return;
  }
  name_len = strlen(name);

  value = config_get_from_index(config);
  if(value == NULL) {
    send_error(UNEXPECTED_ERROR);
    return;
  }
  value_len = strlen(value);


  /** buffer = "(" + config_num(unsigned int) + ")" + name + ": " + value + "\0" */
  msg = malloc(sizeof(msg_t));
  if(msg == NULL) {
    send_error(UNEXPECTED_ERROR);
    return;
  }
  msg->buffer = malloc(name_len + value_len + 15);
  if(msg->buffer == NULL) {
    send_error(UNEXPECTED_ERROR);
    free(msg);
    return;
  }
  msg->type = GET_CONFIG;
  msg->buffer_size = sprintf((char *)msg->buffer, "(%d)%s: %s", config, name, value);
  msg->buffer_size++;

  q_offer(msg);
}
void
check_set_config(unsigned char config, unsigned char * value, int value_len)
{
  msg_t * msg;
  int return_value;
  char * val;

  if(config >= config_get_size()) {
    send_error(CONFIG_NOT_FOUND);
    return;
  }
  if(value_len <= 0) {
    send_error(INVALID_LENGTH);
    return;
  }

  val = malloc((size_t)value_len);
  if(val == NULL) {
    send_error(UNEXPECTED_ERROR);
    return;
  }
  memcpy(val, value, (size_t)value_len);

  free(config_get_from_index(config));

  return_value = config_set_from_index(config, val);
  if(return_value < 0) {
    send_error(CONFIG_NOT_SET);
    return;
  }

  msg = malloc(sizeof(msg_t));
  if(msg == NULL) {
    send_error(UNEXPECTED_ERROR);
    return;
  }
  msg->type = SET_CONFIG;
  msg->buffer_size = 0;

  q_offer(msg);
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