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
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>


int
main(const int argc, const char * argv[])
{
  const char       *err_msg = NULL;
  selector_status  ss       = SELECTOR_SUCCESS;
  fd_selector      selector = NULL;

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port        = htons(ADMIN_PORT);

  if((admin_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    err_msg = "unable to create admin socket";
    /** exit with error */
    printf("%s, error=%d\n", err_msg, errno);
  return 1;
  }

  if(connect(admin_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    err_msg="error al conectar";
    close(admin_socket);
    /** exit with error */
    printf("%s, error=%s\n", err_msg, strerror(errno));
    return 1;
  }

    setsockopt(admin_socket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

  /** Sets non-blocking io on server */

  if(selector_fd_set_nio(admin_socket) == -1) {
    err_msg = "getting server socket flags";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  if(selector_fd_set_nio(STDIN) == -1) {
    err_msg = "getting server socket flags";
    /** exit with error */
    printf("%s\n",err_msg);
    return 1;
  }

  if(selector_fd_set_nio(STDOUT) == -1) {
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

  ss = selector_register(selector, admin_socket, &admin_handler, OP_READ || OP_WRITE, NULL);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering admin_socket";
    /** exit with error */
    return 1;
  }

  ss = selector_register(selector, STDIN, &stdin_handler, OP_READ, NULL);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering stdin";
    /** exit with error */
    return 1;
  }
  ss = selector_register(selector, STDOUT, &stdout_handler, OP_WRITE, NULL);

  if(ss != SELECTOR_SUCCESS) {
    err_msg = "registering stdoutt";
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
  }

}
