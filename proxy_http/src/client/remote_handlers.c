#include "remote_handlers.h"
#include "client_private.h"
#include <client/client.h>
#include <errno.h>
#include <stdio.h>
#include <logger/logger.h>
#include <memory.h>
#include <netdb.h>
#include <response_parser/response_parser.h>
#include <sys/socket.h>

void
remote_read(struct selector_key* key)
{
  client_t client = GET_CLIENT(key);
  switch (client->state) {
    case READ_RESP:
      if (buffer_can_write(&client->pre_res_parse_buf)) {
        size_t buffer_space;
        uint8_t* buffer_ptr =
          buffer_write_ptr(&client->pre_res_parse_buf, &buffer_space);
        ssize_t read_bytes = read(client->origin_fd, buffer_ptr, buffer_space);
        if (read_bytes < 0) {
          printf("Read failed! closing!!!!\n");
          selector_set_interest(client->selector, client->client_fd, OP_WRITE);
          selector_unregister_fd(client->selector, client->origin_fd);
        } else if (read_bytes > 0) {
          printf("Remote read: reading\n");

          buffer_write_adv(&client->pre_res_parse_buf, read_bytes);
          /** Parse the response. The parser dumps pre_res_parse_buf into
           * post_res_parse_buf */
          response_parser_parse(client->response_parser);

          /** Check if it's something to transform */
          if(client->shouldTransform && buffer_can_read (&client->pre_transf_buf)) {
            printf("New characters to transform\n");
            /** Parser added characters to pre transform buffer, so set WRITE interest */
            selector_set_interest(client->selector, client->transf_in_fd, OP_WRITE);
          }

          if(!client->shouldTransform) {
            /** As i wrote to the buffer, write to the client */
            selector_set_interest(client->selector, client->client_fd, OP_WRITE);
          }

        } else {
          selector_unregister_fd(client->selector, client->origin_fd);
        }

      } else {
        /** If buffer is full, stop reading from origin */
        selector_set_interest(client->selector, client->origin_fd, OP_NOOP);
      }
      break;
    default:
      break;
  }
}

void
remote_write(struct selector_key* key)
{
  client_t client = GET_CLIENT(key);

  switch (client->state) {
    case SEND_REQ:
      if (buffer_can_read(&client->post_req_parse_buf)) {
        printf("Sending request..\n");
        size_t buffer_size;
        uint8_t* buffer_ptr =
          buffer_read_ptr(&client->post_req_parse_buf, &buffer_size);
        ssize_t written_bytes =
          write(client->origin_fd, buffer_ptr, buffer_size);
        /** If the read fails, close the connection */
        if (written_bytes == -1) {
          printf("Remote write failed! (%s)\n", strerror(errno));
          selector_set_interest(client->selector, client->client_fd, OP_WRITE);
          selector_unregister_fd(client->selector, client->origin_fd);
          return;
        }

        /** update transfered_bytes metric */
        metric_add_transfered_bytes((double)written_bytes);

        buffer_read_adv(&client->post_req_parse_buf, written_bytes);
        /** As i read from the buffer, read from the client */
        selector_set_interest(client->selector, client->client_fd, OP_READ);
      } else if (client->request_complete) {
        /** If the request is complete, write to the client and read from origin
         */
        selector_set_interest(client->selector, client->origin_fd, OP_READ);
        client->state = READ_RESP;
        printf("Ready sending request!\n");
      } else {
        /** If buffer is empty, stop writing to origin */
        selector_set_interest(client->selector, client->origin_fd, OP_NOOP);
      }
      break;
    default: // dummy
      break;
  }
}

void
remote_block(struct selector_key* key)
{
  /** For now, we won't call blocking jobs on
   *  remote fd. So, nothing to handle */
  return;
}

void
remote_close(struct selector_key* key)
{
  client_t client = GET_CLIENT(key);
  //  shutdown(client->origin_fd,SHUT_RDWR);
  close(client->origin_fd);
  client->origin_fd = -1;
  //  client->state = NO_ORIGIN;
}
