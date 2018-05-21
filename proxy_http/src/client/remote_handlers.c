#include <client/client.h>
#include "remote_handlers.h"
#include "client_private.h"

void
remote_read(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);

  switch(client->state) {
    case READ_RESP:
      /**
       * TODO:
       *    response_parser_parse(client->response_parser,
       *                          key->fd,
       *                          client->out_buffer);
       *
       *    The parser should set the client->response_complete
       *    flag accordingly.
       * */
      break;
    case NO_HOST:break;
    case HOST_RESOLV:break;
    case READ_REQ:break;
    case NO_REMOTE:break;
    case ERROR:break;
  }
}

void
remote_write(struct selector_key * key)
{
  client_t client = GET_CLIENT(key);

  switch(client->state) {
    case READ_REQ:
      if(buffer_can_read(&client->in_buffer)) {
        size_t read_quantity;
        uint8_t * read_ptr = buffer_read_ptr(&client->in_buffer, &read_quantity);
        ssize_t written = write(key->fd, read_ptr, read_quantity);
        if(written > 0) {
          buffer_read_adv(&client->in_buffer, written);
        }
      } else if(client->request_complete) {
        client->state = READ_RESP;
      }
      break;
    case NO_HOST:break;
    case HOST_RESOLV:break;
    case READ_RESP:break;
    case NO_REMOTE:break;
    case ERROR:break;
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

  switch(client->state) {
    case NO_HOST:
    case HOST_RESOLV:
    case READ_REQ:
    case READ_RESP:
    case NO_REMOTE:
    case ERROR:
      selector_unregister_fd(key->s, key->fd);
      client->state = NO_REMOTE;
      break;
  }
}
