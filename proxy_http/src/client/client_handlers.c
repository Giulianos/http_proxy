#include <client/client.h>
#include <buffer/buffer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <netdb.h>
#include <printf.h>
#include <errno.h>
#include <arpa/inet.h>
#include "client_private.h"
#include "remote_handlers.h"

/** Remote handlers */
fd_handler remote_handlers = {
    .handle_read = remote_read,
    .handle_write = remote_write,
    .handle_close = remote_close,
    .handle_block = remote_block,
};

void
client_read(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);
  switch(client->state) {
    case NO_ORIGIN:
      if(buffer_can_write(&client->pre_req_parse_buf)) {
        /** Get the buffer pointer and space available */
        size_t buffer_space;
        uint8_t * buffer_ptr = buffer_write_ptr(&client->pre_req_parse_buf, &buffer_space);
        ssize_t read_bytes = read(client->client_fd, buffer_ptr, buffer_space);
        buffer_write_adv(&client->pre_req_parse_buf, read_bytes);
        /** Parse the request. The parser dumps pre_req_parse_buf into post_req_parse_buf */
        request_parser_parse(client->request_parser);
      } else {
        /** If buffer is full, stop reading from client */
        selector_set_interest(client->selector, client->client_fd, OP_NOOP);
      }
      break;
    case SEND_REQ:
      if(buffer_can_write(&client->pre_req_parse_buf)) {
        /** Get the buffer pointer and space available */
        size_t buffer_space;
        uint8_t * buffer_ptr = buffer_write_ptr(&client->pre_req_parse_buf, &buffer_space);
        ssize_t read_bytes = read(client->client_fd, buffer_ptr, buffer_space);
        buffer_write_adv(&client->pre_req_parse_buf, read_bytes);
        /** Parse the request. The parser dumps pre_req_parse_buf into post_req_parse_buf */
        request_parser_parse(client->request_parser);
        /** As i wrote to the buffer, write to origin */
        selector_set_interest(client->selector, client->origin_fd, OP_WRITE);
      } else {
        /** If buffer is full, stop reading from client */
        selector_set_interest(client->selector, client->client_fd, OP_NOOP);
      }
      break;
  }
}

void
client_write(struct selector_key * key) {
  client_t client = GET_CLIENT(key);

  switch(client->state) {
    case READ_RESP:
      if(buffer_can_read(&client->post_res_parse_buf)) {
        printf("Sending response...\n");
        size_t buffer_size;
        uint8_t * buffer_ptr = buffer_read_ptr(&client->post_res_parse_buf, &buffer_size);
        ssize_t written_bytes = write(client->client_fd, buffer_ptr, buffer_size);
        buffer_read_adv(&client->post_res_parse_buf, written_bytes);
        /** As i read from the buffer, read from the origin */
        selector_set_interest(client->selector, client->origin_fd, OP_READ);
      } else if(client->response_complete) {
        /** If the response is complete, keep-alive! */
        client_restart_state(client);
      }
      break;
  }

}

void
client_block(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);

  /** Connect to remote host */

  int origin_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (origin_socket < 0) {
    client->state = ERROR;
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

  /** Register origin fd to write */
  selector_fd_set_nio(origin_socket);
  selector_register(client->selector, origin_socket, &remote_handlers, OP_WRITE, (void*)client);
  client->state = SEND_REQ;
  client->origin_fd = origin_socket;

  /** Non-blocking connect */

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = inet_addr("216.58.222.46");
  addr.sin_port        = htons(80);

  connect(origin_socket, (const struct sockaddr*)&addr, sizeof(struct sockaddr));
  printf("Connecting to origin...\n");
}

void
client_close(struct selector_key * key)
{
  printf("Bye!\n");
  client_free_resources(GET_CLIENT(key));
}
