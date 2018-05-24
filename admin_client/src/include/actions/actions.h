#ifndef ACTIONS_H
#define ACTIONS_H

#include <msg_queue/msg_queue.h>
#include <messages/messages.h>

void
req_list_metrics();
void
req_list_configs();
void
req_get_metric(char * metric);
void
req_get_config(char * config);
void
req_set_config(char * config, char * value);
  
#endif
