#include <protocol/protocol.h>
#include <handlers/handlers.h>
#include <bits/signum.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <msg_queue/msg_queue.h>
#include <print_queue/print_queue.h>
#include <actions/actions.h>

int
main(const int argc, const char * argv[])
{
  const char *err_msg = NULL;
  int return_value;
  int admin_socket;
  struct sockaddr_in addr;
  struct sockaddr_in peer;
  struct sctp_event_subscribe events;
  struct sctp_sndrcvinfo sri;
  
  if(argc != 3) {
    err_msg = "Usage: admin_client [ip] [port]";
    printf("%s\n", err_msg);
    return 1;
  }

  admin_socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  if(admin_socket < 0) {
    err_msg = "creating socket";
    printf("%s\n", err_msg);
    return 1;
  }
  bzero(&addr, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port        = htons(atoi(argv[2]));
  return_value = inet_pton(AF_INET,argv[1], &addr.sin_addr);
  if(return_value <= 0) {
    err_msg = "inet_pton";
    printf("%s\n", err_msg);
    return 1;
  }
  bzero(&events, sizeof(events));
  events.sctp_data_io_event = 1;
  return_value = setsockopt(admin_socket, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));
  if(return_value < 0) {
    err_msg = "setting socket options";
    printf("%s\n", err_msg);
    return 1;
  }
  bzero(&sri, sizeof(sri));
  sri.sinfo_stream = 0;
  struct addr_data admin_data = {
      .addr       = &addr,
      .addr_len   = sizeof(addr),
      .sri        = &sri,
      .peer       = &peer,
      .peer_len   = sizeof(peer),
  };

  char buffer[MAX_READ];
  static char param1[MAX_READ];
  static char param2[MAX_READ];
  enum state{START = 0, CRED, GMETRIC, GCONFIG, SCONFIG, PROTERROR, CLOSE};
  int should_close = 0;
  int i = 0;
  int j = 0;


  static state = START;
  while(!should_close) {

    switch(state) {
      case START:
        show_menu();
        while(read(STDIN_FILENO, buffer, MAX_READ) < 0) {};
        i = 0; j = 0;
        if(buffer[i] == SEND_CRED+'0') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = CRED;
          break;
        }
        if(buffer[i] == LIST_METRICS+'0') {
          req_list_metrics(&admin_data, admin_socket);
          state = START;
          break;
        }
        if(buffer[i] == LIST_CONFIGS+'0') {
          printf("llamo a list config\n");
          req_list_configs(&admin_data, admin_socket);
          state = START;
          break;
        }
        if(buffer[i] == GET_METRIC+'0') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = GMETRIC;
          break;
        }
        if(buffer[i] == GET_CONFIG+'0') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = GCONFIG;
          break;
        }
        if(buffer[i] == SET_CONFIG+'0') {
          i++;
          while(buffer[i] == ' ') {
            i++;
          }
          state = SCONFIG;
          break;
        }
        if(buffer[i] == CLOSE+'0') {
          state = CLOSE;
          break;
        }

      case CRED:
        j = 0;
        for(; i < MAX_READ && buffer[i] != '\n'; i++) {
          param1[j++] = buffer[i];
        }
        param1[j] = '\0';
        printf("entre a cred\n");
        send_credentials(&admin_data, admin_socket, param1);
        state = START;
        break;
      case GMETRIC:
        j = 0;
        for(; i < MAX_READ && buffer[i] != '\n'; i++) {
          param1[j++] = buffer[i];
        }
        param1[j] = '\0';
        printf("entre a get metric\n");
        req_get_metric(&admin_data, admin_socket, atoi(param1));
        state = START;
        break;
      case GCONFIG:
        j = 0;
        for(i; i < MAX_READ && buffer[i] != '\n'; i++) {
          param1[j++] = buffer[i];
        }
        param1[j] = '\0';
        printf("entre a get config\n");
        req_get_config(&admin_data, admin_socket, atoi(param1));
        state = START;
        break;
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
        printf("entre a set config\n");
        req_set_config(&admin_data, admin_socket, atoi(param1), param2);

        state = START;
        break;
      case CLOSE:
        should_close = 1;
    }
  }
  printf("Adios cliente administrador!\n");
  return 0;
}
