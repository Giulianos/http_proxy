#ifndef ACTIONS_H
#define ACTIONS_H

#include <msg_queue/msg_queue.h>
#include <messages/messages.h>

void
check_credentials(unsigned char * pass, int pass_len);
void
send_list_metrics();
void
send_list_configs();
void
send_metric(unsigned char metric);
void
send_config(unsigned char config);
void
check_set_config(unsigned char config, unsigned char * value, int value_len);


#endif
