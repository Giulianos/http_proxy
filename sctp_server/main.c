#include <netinet/in.h>
#include <netinet/sctp.h>
#include <string.h>
#include <stdio.h>
#include <protocol/protocol.h>
#include <selector/selector.h>
#include <sys/param.h>
#include <bits/signum.h>
#include <config/config.h>
#include <msg_queue/msg_queue.h>
#include "protocol/protocol.h"
#include "messages/messages.h"
#include "serializer/serializer.h"

#define SERVPORT 9090
#define LISTENQ 10

int
main(int argc, char * argv[])
{
  const char       *err_msg = NULL;
  selector_status  ss       = SELECTOR_SUCCESS;
  fd_selector      selector = NULL;

  int admin_socket, msg_flags;
  int return_value;
  struct sockaddr_in serveraddr, cliaddr;
  struct sctp_sndrcvinfo sri;
  struct sctp_event_subscribe evnts;

  admin_socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  if(admin_socket < 0) {
    err_msg = "creating socket";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  bzero(&serveraddr, sizeof(serveraddr));

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(SERVPORT);

  return_value = bind(admin_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  if(return_value < 0) {
    err_msg = "binding socket";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  bzero(&evnts, sizeof(evnts));
  evnts.sctp_data_io_event = 1;
  return_value = setsockopt(admin_socket, IPPROTO_SCTP, SCTP_EVENTS, &evnts, sizeof(evnts));
  if(return_value < 0) {
    err_msg = "setting socket's options";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  return_value = listen(admin_socket, LISTENQ);
  if(return_value < 0) {
    err_msg = "listening socket";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  /** Sets non-blocking io on server */

  if(selector_fd_set_nio(admin_socket) == -1) {
    err_msg = "getting server socket flags";
    /** exit with error */
    printf("%s\n",err_msg);
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

  bzero(&sri, sizeof(sri));
  sri.sinfo_stream = 0;
  struct addr_data admin_data = {
      .addr       = &cliaddr,
      .len        = sizeof(cliaddr),
      .sri        = sri,
      .msg_flags  = msg_flags,
  };

  ss = selector_register(selector, admin_socket, &admin_handler, OP_READ, &admin_data);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering admin_socket";
    /** exit with error */
    return 1;
  }

  q_init(admin_socket, selector);

  config_create("name0", "value0");
  config_create("name1", "value1");
  config_create("name2", "value2");

  for(;;) {
    err_msg = NULL;
    if(q_is_empty()) {
      selector_set_interest(selector, admin_socket, OP_READ);
    }
    ss = selector_select(selector);
    if(!q_is_empty()) {
      printf("Something to read\n");
      selector_set_interest(selector, admin_socket, OP_READ | OP_WRITE);
    }
    if(ss != SELECTOR_SUCCESS) {
      err_msg = "serving";
      /** exit with error */
      return 1;
    }
  }
}