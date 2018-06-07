#include <client/client.h>
#include <sys/socket.h>
#include <memory.h>
#include <errno.h>
#include <netdb.h>
#include <response_parser/response_parser.h>
#include <logger/logger.h>
#include "remote_handlers.h"
#include "client_private.h"

void
remote_read(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);
  switch(client->state) {
    /** This is executed infinitly, check! */
    case READ_RESP:
      if(buffer_can_write(&client->pre_res_parse_buf)) {
        size_t buffer_space;
        uint8_t * buffer_ptr = buffer_write_ptr(&client->pre_res_parse_buf, &buffer_space);
        ssize_t read_bytes = read(client->origin_fd, buffer_ptr, buffer_space);
        if(read_bytes<0) {
            printf("Read failed! closing!!!!\n");
            selector_set_interest(client->client_fd, client->client_fd, OP_WRITE);
            selector_unregister_fd(client->selector,client->origin_fd);
            //selector_unregister_fd(client->selector,client->client_fd);
        } else if(read_bytes>0){
          printf("Remote read: reading\n");

          buffer_write_adv(&client->pre_res_parse_buf, read_bytes);
          /** Parse the response. The parser dumps pre_res_parse_buf into post_res_parse_buf */
//        response_parser_parse(client->response_parser);
          checkResponse(&client->res_data,&client->pre_res_parse_buf,&client->post_res_parse_buf,&client->post_res_parse_buf);
          //TODO check si tira un header coherente
          if(client->res_data.state!=RES_OK && client->res_data.state!=OK){
              printf("RESPONSEPARSER EXPLODED %d",client->state);
          }
          if(client->res_data.parserState==FINISHED){
            selector_set_interest(client->client_fd, client->client_fd, OP_WRITE);
            selector_unregister_fd(client->selector,client->origin_fd);
            return;
          }
          /** As i wrote to the buffer, write to the client */
          selector_set_interest(client->selector, client->client_fd, OP_WRITE);
        }


      } else {
        /** If buffer is full, stop reading from origin */
        selector_set_interest(client->selector, client->origin_fd, OP_NOOP);
      }
      break;
  }

}

void
remote_write(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);

  switch(client->state) {
    case SEND_REQ:
      if(buffer_can_read(&client->post_req_parse_buf)) {

        printf("Sending request..\n");
        size_t buffer_size;
        uint8_t * buffer_ptr = buffer_read_ptr(&client->post_req_parse_buf, &buffer_size);
        ssize_t written_bytes = write(client->origin_fd, buffer_ptr, buffer_size);
        if(written_bytes==-1) {
          printf("Write failed! (%s)\n", strerror(errno));
          exit(43); //TODO ARREGLARd
         }


        buffer_read_adv(&client->post_req_parse_buf, written_bytes);
        /** As i read from the buffer, read from the client */
        selector_set_interest(client->selector, client->client_fd, OP_READ);
      } else if(client->request_complete) {
        /** If the request is complete, write to the client and read from origin */
        selector_set_interest(client->selector, client->origin_fd, OP_READ);
        client->state = READ_RESP;
        printf("Ready sending request!\n");
      } else {
        /** If buffer is empty, stop writing to origin */
        selector_set_interest(client->selector, client->origin_fd, OP_NOOP);
      }
      break;
  }
}

void
remote_block(struct selector_key * key)
{
  /** For now, we won't call blocking jobs on
   *  remote fd. So, nothing to handle */
  return;
}

void
remote_close(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);
  client->origin_fd=-1;
//  selector_unregister_fd(key->s, key->fd);
  client->state = NO_ORIGIN;
}
