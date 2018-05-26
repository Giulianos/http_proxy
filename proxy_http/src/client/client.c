#include <client/client.h>
#include <stdlib.h>
#include <zconf.h>
#include <memory.h>
#include <limits/limits.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include "remote_handlers.h"
#include "client_private.h"

client_t
client_new(const struct client_config * config)
{
  /** TODO:
  *  - create buffers with sizes from config module
  *  - initialize parsers
  */
  client_t client = (client_t)malloc(sizeof(struct client_cdt));

  client->state = NO_ORIGIN;
  client->host.fqdn[0] = '\0';
  client->err = NO_ERROR;
  client->origin_fd = -1;
  client->client_fd = config->fd;
  client->selector = config->selector;

  /** Initialize I/O buffers */
  client->pre_req_parse_buf_mem = malloc(8192);
  buffer_init(&client->pre_req_parse_buf, 8192, client->pre_req_parse_buf_mem);
  client->post_req_parse_buf_mem = malloc(8192);
  buffer_init(&client->post_req_parse_buf, 8192, client->post_req_parse_buf_mem);
  client->pre_res_parse_buf_mem = malloc(8192);
  buffer_init(&client->pre_res_parse_buf, 8192, client->pre_res_parse_buf_mem);
  client->post_res_parse_buf_mem = malloc(8192);
  buffer_init(&client->post_res_parse_buf, 8192, client->post_res_parse_buf_mem);

  /** Initialize request parser */
  struct request_parser_config req_parser_config = {
      .in_buffer = &client->pre_req_parse_buf,
      .out_buffer = &client->post_req_parse_buf,
      .ready_flag = &client->request_complete,
      .data = client,
      .host_found_callback = client_set_host,
  };
  client->request_parser = request_parser_new(&req_parser_config);

  /** Initialize response parser */
  struct response_parser_config res_parser_config = {
      .in_buffer = &client->pre_res_parse_buf,
      .out_buffer = &client->post_res_parse_buf,
      .ready_flag = &client->response_complete,
  };
  client->response_parser = response_parser_new(&res_parser_config);


  return client;
}

void
client_restart_state(client_t client)
{
  /** TODO: Restart client state */
}

void
client_free_resources(client_t client) {

  if(client->origin_fd != -1) {
    close(client->origin_fd);
    selector_unregister_fd(client->selector, client->origin_fd);
    client->origin_fd = -1;
  }

  /**
   * TODO:
   * Resources that should be freed:
   * 1) Parsers
   * 2) Buffers
   * 3) Resolved host
   * 4) Client CDT
   */

}

void
client_terminate(client_t client)
{
  close(client->client_fd);
  selector_unregister_fd(client->selector, client->client_fd);

  client_free_resources(client);
}

void
client_set_host(const char * host, int port, void * data)
{

  client_t client = (client_t)data;
  pthread_t host_resolv_thread;

  /** In case of a keep-alive connection, check if host remains the same */
  if(client->host.fqdn[0] != '\0'){
    if(strncmp(host, client->host.fqdn, MAX_DOMAIN_NAME_LENGTH) != 0 || client->host.port != port) {
      client->err = KEEPALIVE_HOST_NO_MATCH;
      client->state = ERROR;
      return;
    } else {
      /** We are already connected, so we skip to send the request */
      client->state = SEND_REQ;
      return;
    }
  }

  /** Check if the length of the host is valid */
  if(strnlen(host, MAX_DOMAIN_NAME_LENGTH+1) <= MAX_DOMAIN_NAME_LENGTH) {
    strncpy(client->host.fqdn, host, MAX_DOMAIN_NAME_LENGTH);
    client->host.port = port;

    /** Resolve host in separate thread */
    pthread_create(&host_resolv_thread, NULL, request_resolv_blocking, (void *)client);
    return;
  }

  client->err = INVALID_HOST;
  client->state = ERROR;
  return;
}

static void *
request_resolv_blocking(void *data) {
  client_t client = (client_t)data;

  pthread_detach(pthread_self());

  struct addrinfo hints = {
      .ai_family    = AF_INET,    /* Allow IPv4 or IPv6 */
      .ai_socktype  = SOCK_STREAM,  /* Datagram socket */
      .ai_flags     = AI_PASSIVE,   /* For wildcard IP address */
      .ai_protocol  = 0,            /* Any protocol */
      .ai_canonname = NULL,
      .ai_addr      = NULL,
      .ai_next      = NULL,
  };

  char buff[7];
  snprintf(buff, sizeof(buff), "%d",
           ntohs(client->host.port));

  getaddrinfo(client->host.fqdn, buff, &hints,
              &client->host.resolved);

  selector_notify_block(client->selector, client->client_fd);

  free(data);

  return 0;
}