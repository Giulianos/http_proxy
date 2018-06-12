#include "client_private.h"
#include "remote_handlers.h"
#include "transformation_handlers.h"
#include <arpa/inet.h>
#include <client/client.h>
#include <limits/limits.h>
#include <logger/logger.h>
#include <memory.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <transformations/transformations.h>
#include <config/config.h>

static void* request_resolv_blocking(void* data);

/** Transformation handlers */
fd_handler transf_in_handlers = {
    .handle_read  = NULL,
    .handle_write = transf_write,
    .handle_close = transf_close,
    .handle_block = transf_block,
};

fd_handler transf_out_handlers = {
    .handle_read  = transf_read,
    .handle_write = NULL,
    .handle_close = transf_close,
    .handle_block = transf_block,
};

client_t
client_new(const struct client_config* config)
{
  client_t client = (client_t)malloc(sizeof(struct client_cdt));

  client->state = NO_ORIGIN;
  client->resolved = NULL;
  client->err = NO_ERROR;
  client->origin_fd = -1;
  client->client_fd = config->fd;
  client->selector = config->selector;

  /** Transformation setup */
  client->shouldTransform = false;
  transformations_new(&client->transf_in_fd, &client->transf_out_fd);

  if(client->shouldTransform) {
    selector_register (client->selector, client->transf_in_fd, &transf_in_handlers, OP_NOOP, client);
    selector_register (client->selector, client->transf_out_fd, &transf_out_handlers, OP_NOOP, client);
  }

  size_t buffer_size = (size_t)atoi(config_get("buffers_size"));
  size_t reserved_space = 255;

  /** Initialize I/O buffers */
  client->pre_req_parse_buf_mem = malloc(buffer_size);
  buffer_init_r(&client->pre_req_parse_buf, reserved_space, buffer_size,
                client->pre_req_parse_buf_mem);
  client->post_req_parse_buf_mem = malloc(buffer_size);
  buffer_init_r(&client->post_req_parse_buf, reserved_space, buffer_size,
                client->post_req_parse_buf_mem);
  client->pre_res_parse_buf_mem = malloc(buffer_size);
  buffer_init_r(&client->pre_res_parse_buf, reserved_space, buffer_size,
                client->pre_res_parse_buf_mem);
  client->post_res_parse_buf_mem = malloc(buffer_size);
  buffer_init_r(&client->post_res_parse_buf, reserved_space, buffer_size,
                client->post_res_parse_buf_mem);
  client->pre_transf_buf_mem = malloc(buffer_size);
  buffer_init_r(&client->pre_transf_buf, reserved_space, buffer_size,
                client->pre_transf_buf_mem);

  /** Initialize request parser */
  struct request_parser_config req_parser_config = {
      .in_buffer =  &client->pre_req_parse_buf,
      .out_buffer = &client->post_req_parse_buf,
      .got_host_callback = client_set_host,
      .request_ended_callback = request_ended,
      .callbacks_info = (void *)client
  };
  client->request_parser = request_parser_new(&req_parser_config);

  /** Initialize response parser */
  struct response_parser_config res_parser_config = {
      .in_buffer =  &client->pre_res_parse_buf,
      .out_buffer = &client->post_res_parse_buf,
      .trans_buffer = &client->pre_transf_buf,
      .response_ended_callback = response_ended,
      .callbacks_info = (void *)client
  };
  client->response_parser = response_parser_new(&res_parser_config);

  /** add client's metrics */
  client->connection_time = metric_new_connection();

  return client;
}

void
client_free_resources(client_t client)
{
  /** Unregister origin */
  if (client->origin_fd != -1) {
    selector_unregister_fd(client->selector, client->origin_fd);
  }

  /** Close client */
  close(client->client_fd);
  client->client_fd = -1;

  /** Destroy parsers */
  request_parser_destroy (client->request_parser);

  /** Free buffers */
  if(client->pre_req_parse_buf_mem != NULL)
    free(client->pre_req_parse_buf_mem);
  if(client->post_req_parse_buf_mem != NULL)
    free(client->post_req_parse_buf_mem);
  if(client->pre_res_parse_buf_mem != NULL)
    free(client->pre_res_parse_buf_mem);
  if(client->post_req_parse_buf_mem != NULL)
    free(client->post_res_parse_buf_mem);
  if(client->pre_transf_buf_mem != NULL)
    free(client->pre_transf_buf_mem);

  if(client->resolved != NULL)
    freeaddrinfo (client->resolved);

  free(client);
}

void
client_set_host(host_details_t host, void* data)
{

  client_t client = (client_t)data;
  pthread_t host_resolv_thread;
  log_sendf(client->log, "Resolving: %s...", host->host);

  /** Check if the length of the host is valid */
  if (strnlen(host->host, MAX_DOMAIN_NAME_LENGTH + 1) <= MAX_DOMAIN_NAME_LENGTH) {
    strncpy(client->host, host->host, MAX_DOMAIN_NAME_LENGTH);
    client->port = host->port;

    /** Resolve host in separate thread */
    pthread_create(&host_resolv_thread, NULL, request_resolv_blocking,
                   (void*)client);
    return;
  }

  client->err = INVALID_HOST;
  client->state = CLI_ERROR;
}

void
request_ended(void* data)
{
  client_t client = (client_t)data;

  client->request_complete = true;
}

void
response_ended(void* data)
{
  client_t client = (client_t)data;
  dprintf(STDERR_FILENO, "[CLIENT_INFO]Response ended!\n");
  client->response_complete = true;
}

static void*
request_resolv_blocking(void* data)
{
  client_t client = (client_t)data;

  pthread_detach(pthread_self());

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,     /* Allow IPv4 or IPv6 */
    .ai_socktype = SOCK_STREAM, /* Datagram socket */
    .ai_flags = AI_PASSIVE,     /* For wildcard IP address */
    .ai_protocol = 0,           /* Any protocol */
    .ai_canonname = NULL,
    .ai_addr = NULL,
    .ai_next = NULL,
  };

  getaddrinfo(client->host, NULL, &hints, &client->resolved);
  selector_notify_block(client->selector, client->client_fd);

  return 0;
}
