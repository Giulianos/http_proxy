#include <netinet/in.h>
#include <netinet/sctp.h>
#include <string.h>
#include <stdio.h>
#include <protocol/protocol.h>
#include <selector/selector.h>
#include <sys/param.h>
#include <bits/signum.h>
#include "protocol/protocol.h"
#include "messages/messages.h"
#include "serializer/serializer.h"

#define BUFFSIZE 800
#define SERVPORT 9090
#define LISTENQ 10

int
main(int argc, char * argv[])
{
  const char       *err_msg = NULL;
  selector_status  ss       = SELECTOR_SUCCESS;
  fd_selector      selector = NULL;

  int admin_socket, msg_flags;
  struct sockaddr_in serveraddr, cliaddr;
  struct sctp_sndrcvinfo sri;
  struct sctp_event_subscribe evnts;

  admin_socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);

  bzero(&serveraddr, sizeof(serveraddr));

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(SERVPORT);

  bind(admin_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

  bzero(&evnts, sizeof(evnts));
  evnts.sctp_data_io_event = 1;
  setsockopt(admin_socket, IPPROTO_SCTP, SCTP_EVENTS, &evnts, sizeof(evnts));

  listen(admin_socket, LISTENQ);

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

  ss = selector_register(selector, admin_socket, &admin_handler, OP_READ | OP_WRITE, &admin_data);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering admin_socket";
    /** exit with error */
    return 1;
  }


  for(;;) {
    err_msg = NULL;
    ss = selector_select(selector);
    if(ss != SELECTOR_SUCCESS) {
      err_msg = "serving";
      /** exit with error */
      return 1;
    }

//    rd_sz = sctp_recvmsg(admin_socket, readbuf, sizeof(readbuf),
//        (struct sockaddr *)&cliaddr, &len, &sri, &msg_flags);
//    if(rd_sz > 0) {
//      msg_t msg;
//      deserialize_msg(readbuf, &msg);
//      printf("mensaje recibido: \n");
//      printf(" type: %d \n", msg.type);
//      printf(" param: %d \n", msg.param);
//      printf(" buf_size: %d \n", msg.buffer_size);
//      if(msg.buffer_size > 0)
//        printf(" buffer: %s \n", msg.buffer);
//      //print_msg(&msg);
//    }

//    sctp_sendmsg(admin_socket, readbuf, rd_sz,
//                 (struct sockaddr *)&cliaddr,
//                     len, sri.sinfo_ppid, sri.sinfo_flags,
//                 sri.sinfo_stream, 0, 0);
  }
}