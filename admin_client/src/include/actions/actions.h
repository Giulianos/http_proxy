#ifndef ACTIONS_H
#define ACTIONS_H

#include <messages/messages.h>

void send_credentials(addr_data_t servdata, int socket, const char* pass);
void req_list_metrics(addr_data_t servdata, int socket);
void req_list_configs(addr_data_t servdata, int socket);
void req_get_metric(addr_data_t servdata, int socket, unsigned char metric);
void req_get_config(addr_data_t servdata, int socket, unsigned char config);
void req_set_config(addr_data_t servdata, int socket, unsigned char config,
                    unsigned char* value);
void show_menu();
void get_and_show_response(addr_data_t servdata, int socket, int is_list);

#endif
