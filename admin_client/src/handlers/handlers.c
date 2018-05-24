#include <handlers/handlers.h>
#include <print_queue/print_queue.h>
#include <actions/actions.h>
#include <messages/messages.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

static int is_first_time = 1;
void
admin_read_handler()
{
  int i = 0;
  msg_t * msg = malloc(MAX_READ);

  rcv_msg(admin_socket, msg);

  if(msg->type == SET_CONFIG) {
    pq_offer(msg->param);
    return;
  }
  if(msg->type == GET_METRIC ||
     msg->type == GET_CONFIG){
    pq_offer(msg->param);
  }
  pq_offer(msg->buffer);
}

void
admin_write_handler() {
  int written_bytes;
  int i = 0;

  if (q_is_empty())
    return;

  qnode_t qnode = malloc(sizeof(qnode_t));
  qnode = poll();

  written_bytes = send_msg(admin_socket, qnode->msg);

  /**missing if the message was not sent completely*/
//  if(written_bytes < qnode->msg->bytes) {
//    qnode->msg->bytes -= written_bytes;
//    /** consume bytes already written before pushing the msg */
//    }
}
void
stdin_read_handler()
{
  int i = 0;
  int j = 0;
  ssize_t bytes;

  char buffer[MAX_READ];
  static char param1[MAX_READ];
  static char param2[MAX_READ];

  enum state{START, GMETRIC, GCONFIG, SCONFIG};

  static state = START;
  bytes = read(STDIN, buffer, MAX_READ);

  while(i < bytes) {
    switch(state) {
      case START:
        if(buffer[i] == '0') {
          req_list_metrics();
          state = START;
          return;
        }
        if(buffer[i] == '1') {
          req_list_configs();
          state = START;
          return;
        }
        if(buffer[i] == '2') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = GMETRIC;
          break;
        }
        if(buffer[i] == '3') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = GCONFIG;
          break;
        }
        if(buffer[i] == '4') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = SCONFIG;
          break;
        }
      case GMETRIC:
        j = 0;
        for(i; i < MAX_READ && buffer[i] != '\n'; i++) {
          param1[j++] = buffer[i];
        }
        param1[j] = '\0';
        req_get_metric(param1);
        state = START;
        return;
      case GCONFIG:
        j = 0;
        for(i; i < MAX_READ && buffer[i] != '\n'; i++) {
          param1[j++] = buffer[i];
        }
        param1[j] = '\0';
        req_get_config(param1);
        state = START;
        return;
      case SCONFIG:
        j = 0;
        for(i; i < MAX_READ && buffer[i] != ' '; i++) {
          param1[j++] = buffer[i];
        }
        param1[j++] = '\0';
        i++;
        while(buffer[i] == ' ') {
          i++;
        }

        j = 0;
        for(i; i < MAX_READ && buffer[i] != '\n'; i++) {
          param2[j++] = buffer[i];
        }
        param2[j] = '\0';

        req_set_config(param1, param2);

        state = START;
        return;

        /** si no entra a los anteriores, retorno porque no es valido*/
      default:
        return;
    }

  }
}
void
stdout_write_handler()
{
  unsigned char * str;

  if(is_first_time) {
    is_first_time = 0;
    write(STDOUT, "Bienvenido Administrador\n"
                  "0) Listar metricas\n"
                  "1) Listar configuraciones\n"
                  "2) Obtener metrica (indicar nombre de metrica)\n"
                  "3) Obtener configuracion (indicar nombre de configuracion)\n"
                  "4) Setear configuracion (indicar nombre de configuracion y valor deseado\n"
                  "5) Cerrar\n", 260);
    return;
  }
  if(pq_is_empty())
    return;
  while(!pq_is_empty()) {
    str = pq_poll();
    write(STDOUT, str, strlen(str));
    free(str);
  }
}