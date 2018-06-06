#include <actions/actions.h>
#include <messages/messages.h>
#include <protocol/protocol.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void
send_credentials(addr_data_t servdata, int socket, const char * pass)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = SEND_CRED;
  msg->param = 0;
  msg->buffer_size = (unsigned int)strlen(pass)+1;
  msg->buffer = malloc(msg->buffer_size);
  strncpy((char *)msg->buffer, pass, msg->buffer_size);

  send_msg(servdata, socket, msg);
}

void
req_list_metrics(addr_data_t servdata, int socket)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = LIST_METRICS;
  msg->param = 0;
  msg->buffer_size = 0;

  send_msg(servdata, socket, msg);
  get_and_show_response(servdata, socket, 1);
}

void
req_list_configs(addr_data_t servdata, int socket)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = LIST_CONFIGS;
  msg->param = 0;
  msg->buffer_size = 0;

  send_msg(servdata, socket, msg);
  get_and_show_response(servdata, socket, 1);
}

void
req_get_metric(addr_data_t servdata, int socket, unsigned char metric)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = GET_METRIC;
  msg->param = metric;
  msg->buffer_size = 0;

  send_msg(servdata, socket, msg);
  get_and_show_response(servdata, socket, 0);
}

void
req_get_config(addr_data_t servdata, int socket, unsigned char config)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = GET_CONFIG;
  msg->param = config;
  msg->buffer_size = 0;

  send_msg(servdata, socket, msg);
  get_and_show_response(servdata, socket, 0);
}

void
req_set_config(addr_data_t servdata, int socket, unsigned char config, unsigned char * value)
{
  msg_t * msg = malloc(sizeof(msg_t));
  msg->type = SET_CONFIG;
  msg->param = config;
  msg->buffer_size = (unsigned int)strlen((char *)value)+1;
  msg->buffer = malloc(msg->buffer_size);
  strncpy((char *)msg->buffer, (char *)value, msg->buffer_size);

  send_msg(servdata, socket, msg);
  get_and_show_response(servdata, socket, 0);
}

void
show_menu()
{
  printf("Bienvenido Administrador\n"
         "0) Enviar credenciales (password)\n"
         "1) Listar metricas\n"
         "2) Listar configuraciones\n"
         "3) Obtener metrica (indicar numero de metrica)\n"
         "4) Obtener configuracion (indicar numero de configuracion)\n"
         "5) Setear configuracion (indicar numero de configuracion y valor deseado\n"
         "6) Cerrar\n");
}

void
get_and_show_response(addr_data_t servdata, int socket, int is_list)
{
  msg_t response;
  ssize_t rcv_sz;
  do {
    rcv_sz = rcv_msg(servdata, socket, &response);
    if(rcv_sz < 0) {
      printf("error receiving\n");
      return;
    }
    print_msg(&response);
  } while(is_list && response.buffer_size > 0);

}