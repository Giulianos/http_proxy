#include <client/client.h>
#include <buffer/buffer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <netdb.h>
#include "client_private.h"
#include "remote_handlers.h"

#define GET_CLIENT(key) (client_t)((key)->data)

void
client_read(const struct selector_key * key)
{
  client_t client = GET_CLIENT(key);

  switch(client->state) {
    case NO_HOST:
    case READ_REQ:
      /**
       * request_parser_parse(request_parser);
       *
       * A callback should be called when the host is found,
       * in this case is client_set_host.
       *
       * */
      break;
    case NO_REMOTE:
      /**
       * http_response_build(503, out_buffer, &response_complete);
       * */
      selector_set_interest(key->s, key->fd, OP_WRITE);
      break;
    case ERROR:
      /**
       * Check error code (client->err) and build apropiate response
       *
       * http_response_build(5xx, out_buffer, &response_complete);
       * */
      selector_set_interest(key->s, key->fd, OP_WRITE);
      break;
    /** In any other state, we do nothing */
    case HOST_RESOLV:
    case READ_RESP:break;
  }
}

void
client_write(const struct selector_key * key) {
  client_t client = GET_CLIENT(key);

  /** TODO:
   * - check error state */

  /** If there are chars in the output buffer we send them to the client */
  if(buffer_can_read(&client->out_buffer)) {
    size_t read_quantity;
    uint8_t * read_ptr = buffer_read_ptr(&client->out_buffer, &read_quantity);
    ssize_t written = write(key->fd, (const void *)read_ptr, read_quantity);
    if(written > 0) {
      buffer_read_adv(&client->out_buffer, (size_t)written);
    }
  } else if(client->response_complete) {
    switch (client->state) {
      case READ_RESP:
        /** Go to intial state, to support keep-alive */
        client_restart_state(client);
        break;
      case NO_REMOTE:
        /** Terminate the client */
        client_terminate(client);
        break;
      /** In any other state, we do nothing */
      case NO_HOST:
      case HOST_RESOLV:
      case READ_REQ:
      case ERROR:
        break;
    }
  }
}

void
client_block(const struct selector_key * key)
{
  client_t client = GET_CLIENT(key);

  /** Connect to remote host */

  int remote_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (remote_sock < 0) {
    client->state = NO_HOST;
    return;
  }

  /**
   *
   * TODO:
   * Have an array of resolved ip's,
   * then check with each one if the
   * connection succeeds.
   *
   */
  struct sockaddr_in remote;
  memset(&remote, 0, sizeof(remote));

  remote.sin_family      = AF_INET;
  remote.sin_port        = htons(client->host.port);
  remote.sin_addr.s_addr = *((unsigned long *)client->host.hostnm->h_addr_list[0]);


  /** Non-blocking connect */
  selector_fd_set_nio(remote_sock);

  selector_register(client->selector, remote_sock, &remote_handlers, OP_WRITE, (void*)client);

  connect(remote_sock, (struct sockaddr *)&remote, sizeof(remote));

}

void
client_close(const struct selector_key * key)
{
  client_free_resources(GET_CLIENT(key));
}
