#include <handlers/handlers.h>
#include <print_queue/print_queue.h>
#include <actions/actions.h>
#include <messages/messages.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <protocol/protocol.h>

static int is_first_time = 1;
void
admin_read_handler(struct selector_key * key)
{
  int i = 0;
  addr_data_t servdata = (addr_data_t) key->data;
  msg_t * msg = malloc(sizeof(msg_t));

  rcv_msg(servdata, key->fd, msg);

  pq_offer(msg);
}

void
admin_write_handler(struct selector_key * key) {
  msg_t * msg;
  addr_data_t servdata = (addr_data_t) key->data;

  if (q_is_empty())
    return;

  msg = poll();

  send_msg(servdata, key->fd, msg);
}
/** MISSING ERROR MANAGEMENT AND SEND CREDENTIALS */
void
stdin_read_handler(struct selector_key * key)
{
  int i = 0;
  int j = 0;
  ssize_t bytes;

  char buffer[MAX_READ];
  static char param1[MAX_READ];
  static char param2[MAX_READ];

  enum state{START, CRED, GMETRIC, GCONFIG, SCONFIG, PROTERROR};

  static state = START;
  bytes = read(key->fd, buffer, MAX_READ);

  while(i < bytes) {
    switch(state) {
      case START:
        if(buffer[i] == '0') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = CRED;
          break;
        }
        if(buffer[i] == '1') {
          req_list_metrics();
          state = START;
          return;
        }
        if(buffer[i] == '2') {
          req_list_configs();
          state = START;
          return;
        }
        if(buffer[i] == '3') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = GMETRIC;
          break;
        }
        if(buffer[i] == '4') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = GCONFIG;
          break;
        }
        if(buffer[i] == '5') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = SCONFIG;
          break;
        }
        if(buffer[i] == '6') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          break;
        }

      case CRED:
        j = 0;
        for(i; i < MAX_READ && buffer[i] != '\n'; i++) {
          param1[j++] = buffer[i];
        }
        param1[j] = '\0';
        send_credentials(param1);
        state = START;
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
      case PROTERROR:
        j = 0;
        for(i; i < MAX_READ && buffer[i] != '\n'; i++) {
          param1[j++] = buffer[i];
        }
        param1[j] = '\0';
        error_handler(param1);
        return;

        /** si no entra a los anteriores, retorno porque no es valido*/
      default:
        return;
    }

  }
}
void
stdout_write_handler(struct selector_key * key)
{
  msg_t * msg;

  if(is_first_time) {
    is_first_time = 0;
    write(key->fd, "Bienvenido Administrador\n"
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
    msg = pq_poll();
    print_msg(msg);

    if(msg->buffer_size > 0) {
      free(msg->buffer);
    }
    free(msg);
  }
}