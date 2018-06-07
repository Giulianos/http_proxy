#include <client/client.h>
#include <buffer/buffer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <netdb.h>
#include <printf.h>
#include <errno.h>
#include <arpa/inet.h>
#include <requestParser/requestParser.h>
#include <logger/logger.h>
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
client_read(struct selector_key * key);
void
client_write(struct selector_key * key);
void
client_block(struct selector_key * key);
void
client_close(struct selector_key * key);

fd_handler client_handlers2 = {
        .handle_read = client_read,
        .handle_write = client_write,
        .handle_close = client_close,
        .handle_block = client_block,
};


void
client_read(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);
  switch(client->state) {
    case NO_ORIGIN: //solo entra aca
      if(buffer_can_write(&client->pre_req_parse_buf)) {
        /** Get the buffer pointer and space available */
        size_t buffer_space;
        uint8_t * buffer_ptr = buffer_write_ptr(&client->pre_req_parse_buf, &buffer_space);

        ssize_t read_bytes = read(client->client_fd, buffer_ptr, buffer_space);
        buffer_write_adv(&client->pre_req_parse_buf, read_bytes);
        /** Parse the request. The parser dumps pre_req_parse_buf into post_req_parse_buf */
        if(client->req_data.parserState!=FINISHED)
          client->request_complete = checkRequest(&client->req_data, &client->pre_req_parse_buf,
                                                  &client->post_req_parse_buf, client_set_host, client);
        if(client->req_data.state!=OK){
          selector_unregister_fd(client->selector,client->client_fd);
          printf("COMANDO INVALIDO----\n");
          return; //cosas sin sentido
        }
        while(readAndWrite(&client->pre_req_parse_buf,&client->post_req_parse_buf)); //TODO parche groncho

        //    client->state= (client_state_t) client->req_data.state;



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
        client->request_complete = checkRequest(&client->req_data, &client->pre_req_parse_buf,
                                                &client->post_req_parse_buf, client_set_host, client);
          while(readAndWrite(&client->pre_req_parse_buf,&client->post_req_parse_buf)); //TODO parche groncho
          client->state= (client_state_t) client->req_data.state;


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
  struct addrinfo * res = client->host.resolved;
  int status;
  int origin_socket;

  struct addrinfo * current = res;
  char addr_str[50];

  /** Connect to remote host */

  origin_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
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

  printf("Trying connect...\n");
  selector_fd_set_nio(origin_socket);
  //selector_register(client->selector, origin_socket, &remote_handlers, OP_WRITE, (void*)client);
  client->state = SEND_REQ;
  client->origin_fd = origin_socket;

  /** Set port accordingly, now hardcoded */
  if(res->ai_family == AF_INET) {
    ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(client->host.port);
  } else if(res->ai_family == AF_INET6) {
    ((struct sockaddr_in6 *)res->ai_addr)->sin6_port = htons(client->host.port);
  }


  /** Non-blocking connect */

  status = connect(origin_socket, res->ai_addr, res->ai_addrlen);

  if(status == 0) {
      printf("Connection to origin initiated!\n");
  }else if(errno==EINPROGRESS){ //si la conexion esta en proceso
      selector_status st = selector_set_interest_key(key, OP_NOOP);
      if(SELECTOR_SUCCESS != st) {
        printf("Fail to register in select");
        exit(13);
      }

      st = selector_register(key->s, client->origin_fd, &remote_handlers, OP_WRITE, (void*)client);
      if(SELECTOR_SUCCESS != st) {
          printf("Fail to register in select");
          exit(13);
      }
      printf("Connected!\n");
  }  else {
    log_sendf(client->log,"Connection failed! (%s)\n", strerror(errno));
  }

}

void
client_close(struct selector_key * key)
{
  printf("Bye!\n");
  client_free_resources(GET_CLIENT(key));
}