#include <buffer/buffer.h>
#include <limits/limits.h>


enum client_state {
    NO_HOST,
    HOST_RESOLV,
    READ_REQ,
    READ_RESP,
    NO_REMOTE,
    ERROR,
};

typedef enum client_state client_state_t;

struct host_details {
    unsigned int port;
    char name[MAX_DOMAIN_NAME_LENGTH+1];
    struct hostent *hostnm;
};

struct client_cdt {
    client_state_t state;
    client_error_t err;
    bool request_complete;
    bool response_complete;
    buffer in_buffer;
    buffer out_buffer;
    int client_fd;
    int remote_fd;
    fd_selector selector;
    struct host_details host;
};


void
client_restart_state(client_t client);

void
client_free_resources(client_t client);

void
client_terminate(client_t client);

int
client_set_host(client_t client, const char * host, unsigned int port);
