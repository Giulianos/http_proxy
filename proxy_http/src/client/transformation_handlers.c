#include "transformation_handlers.h"
#include "client_private.h"

void
transf_read(struct selector_key * key)
{
  client_t client = GET_CLIENT (key);

  if(buffer_can_write (&client->post_res_parse_buf)) {
    printf("Getting transformation...\n");
    ssize_t read_bytes = dump_chunk_from_fd (client->transf_out_fd, &client->post_res_parse_buf);
    if( read_bytes < 0 ) {
      printf("Error reading from transformation program\n");
      selector_unregister_fd (client->selector, client->client_fd);
    } else if(read_bytes == 0) {
      if(write_empty_chunk (&client->post_res_parse_buf) > 0) {
        printf("Empty chunk sent!\n");
        selector_unregister_fd (client->selector, client->transf_out_fd);
      }
    }
  } else {
    /** If buffer is full, stop reading */
    selector_set_interest(client->selector, client->transf_out_fd, OP_NOOP);
  }
}


void
transf_write(struct selector_key * key)
{
  client_t client = GET_CLIENT (key);

  if(buffer_can_read (&client->pre_transf_buf)) {
    printf("Transforming...\n");
    size_t buffer_size;
    uint8_t* buffer_ptr =
        buffer_read_ptr(&client->pre_transf_buf, &buffer_size);
    ssize_t written_bytes =
        write(client->transf_in_fd, buffer_ptr, buffer_size);
    if(written_bytes < 0) {
      selector_unregister_fd (client->selector, client->transf_in_fd);
    } else if(written_bytes > 0) {
      buffer_read_adv (&client->pre_transf_buf, written_bytes);
    } else {
      selector_unregister_fd (client->selector, client->transf_in_fd);
    }
  } else if(client->res_data.parserState == RES_FINISHED) {
    selector_unregister_fd (client->selector, client->transf_in_fd);
  } else {
    /** If buffer is empty, stop writing */
    selector_set_interest(client->selector, client->transf_in_fd, OP_NOOP);
  }
}


void
transf_block(struct selector_key * key)
{
  /** Do nothing */
}

void
transf_close(struct selector_key * key)
{
  client_t client = GET_CLIENT (key);
  if(key->fd == client->transf_out_fd) {
    close(key->fd);
    client->transf_out_fd = -1;
  } else if(key->fd == client->transf_in_fd) {
    close(key->fd);
    client->transf_in_fd = -1;
  }
}