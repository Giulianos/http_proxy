#ifndef ACTIONS_H
#define ACTIONS_H

#include <msg_queue/msg_queue.h>
#include <messages/messages.h>

void
send_credentials(unsigned char * pass);
void
req_list_metrics();
void
req_list_configs();
void
req_get_metric(unsigned char metric);
void
req_get_config(unsigned char config);
void
req_set_config(unsigned char config, unsigned char * value);
void
error_handler(unsigned char error_type);

  
#endif
