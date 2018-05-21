#include <client/client.h>
#include <stdlib.h>
#include <zconf.h>
#include <memory.h>
#include <limits/limits.h>
#include "remote_handlers.h"
#include "client_private.h"

client_t
client_new(const struct client_config * config)
{
  client_t client = (client_t)malloc(sizeof(struct client_cdt));

  client->state = NO_HOST;
  client->host.name[0] = '\0';
  client->err = NO_ERROR;
  client->remote_fd = -1;
  client->client_fd = config->fd;
  client->selector = config->selector;
  /** TODO:
   *  - create buffers with sizes from config module
   *  - initialize parsers
   */

  return client;
}

void
client_restart_state(client_t client)
{
  /** TODO: Restart client state */
}

void
client_free_resources(client_t client) {

  if(client->remote_fd != -1) {
    close(client->remote_fd);
    selector_unregister_fd(client->selector, client->remote_fd);
    client->remote_fd = -1;
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

int
client_set_host(client_t client, const char * host, unsigned int port)
{

    /** In case of a keep-alive connection, check if host remains the same */
    if(client->host.name[0] != '\0'){
        if(strncmp(host, client->host.name, MAX_DOMAIN_NAME_LENGTH) != 0 || client->host.port != port) {
            client->err = KEEPALIVE_HOST_NO_MATCH;
            client->state = ERROR;
            return -1;
        } else {
            /** We are already connected, so we skip to read the request */
            client->state = READ_REQ;
            return 0;
        }
    }

    /** Check if the length of the host is valid */
    if(strnlen(host, MAX_DOMAIN_NAME_LENGTH+1) <= MAX_DOMAIN_NAME_LENGTH) {
        strncpy(client->host.name, host, MAX_DOMAIN_NAME_LENGTH);
        client->host.port = port;
        client->state = HOST_RESOLV;
        /** TODO: create thread to resolve host */
        return 0;
    }

    client->err = INVALID_HOST;
    client->state = ERROR;
    return -1;
}
