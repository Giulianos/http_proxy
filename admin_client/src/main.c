#include <selector/selector.h>
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

int
main(const int argc, const char * argv[])
{
  const char       *err_msg = NULL;
  selector_status  ss       = SELECTOR_SUCCESS;
  fd_selector      selector = NULL;


  int admin_socket;
  struct sockaddr_in addr;
  struct sctp_event_subscribe events;
  struct sctp_sndrcvinfo sri;

  admin_socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  bzero(&addr, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port        = htons(ADMIN_PORT);
  inet_pton(AF_INET,"127.0.0.1", &addr.sin_addr);

  bzero(&events, sizeof(events));
  events.sctp_data_io_event = 1;
  setsockopt(admin_socket, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));

  /** Sets non-blocking io on admin_socket */

  if(selector_fd_set_nio(admin_socket) == -1) {
    err_msg = "getting server socket flags";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  if(selector_fd_set_nio(STDIN_FILENO) == -1) {
    err_msg = "getting server socket flags";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  if(selector_fd_set_nio(STDOUT_FILENO) == -1) {
    err_msg = "getting server socket flags";
    /** exit with error */
    return 1;
  }

  const struct selector_init conf = {
          .signal = SIGALRM,
          .select_timeout = {
                  .tv_sec  = 10,
                  .tv_nsec = 0,
          },
  };

  if(selector_init(&conf) != 0) {
      err_msg = "initializing selector";
      /** exit with error */
      return 1;
  }

  selector = selector_new(1024);
  if(selector == NULL) {
      err_msg = "unable to create selector";
      /** exit with error */
      return 1;
  }

  const struct fd_handler admin_handler = {
          .handle_read       = admin_read_handler,
          .handle_write      = admin_write_handler,
          .handle_close      = NULL, // nada que liberar
          .handle_block      = NULL,
  };

  const struct fd_handler stdin_handler = {
      .handle_read       = stdin_read_handler,
      .handle_write      = NULL,
      .handle_close      = NULL, // nada que liberar
      .handle_block      = NULL,
  };

  const struct fd_handler stdout_handler = {
      .handle_read       = NULL,
      .handle_write      = stdout_write_handler,
      .handle_close      = NULL, // nada que liberar
      .handle_block      = NULL,
  };

  bzero(&sri, sizeof(sri));
  sri.sinfo_stream = 0;
  struct addr_data admin_data = {
      .addr       = &addr,
      .len        = sizeof(addr),
      .sri        = sri,
  };

  ss = selector_register(selector, admin_socket, &admin_handler, OP_READ | OP_WRITE, &admin_data);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering admin_socket";
    /** exit with error */
    return 1;
  }

  ss = selector_register(selector, STDIN_FILENO, &stdin_handler, OP_READ, NULL);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering stdin";
    /** exit with error */
    return 1;
  }
  ss = selector_register(selector, STDOUT_FILENO, &stdout_handler, OP_WRITE, NULL);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering stdoutt";
    /** exit with error */
    return 1;
  }

  printf("Bienvenido Administrador\n"
                 "0) Enviar credenciales (password)\n"
                 "1) Listar metricas\n"
                 "2) Listar configuraciones\n"
                 "3) Obtener metrica (indicar numero de metrica)\n"
                 "4) Obtener configuracion (indicar numero de configuracion)\n"
                 "5) Setear configuracion (indicar numero de configuracion y valor deseado\n"
                 "6) Cerrar\n");

  for(;;) {
      err_msg = NULL;
      if(q_is_empty()) {
        selector_set_interest(selector, admin_socket, OP_READ);
      }
      if(pq_is_empty()) {
        selector_set_interest(selector, STDOUT_FILENO, OP_NOOP);
      }
      ss = selector_select(selector);
      if(!q_is_empty()) {
        selector_set_interest(selector, admin_socket, OP_READ | OP_WRITE);
      }
      if(!pq_is_empty()) {
        selector_set_interest(selector, STDOUT_FILENO, OP_WRITE);
      }
      if(ss != SELECTOR_SUCCESS) {
          err_msg = "serving";
          /** exit with error */
          return 1;
      }
  }

}
