#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include <selector/selector.h>

void
listen_read_handler(struct selector_key *key);
void
client_block_handler(struct selector_key *key);
void
client_read_handler(struct selector_key *key);
void
example_blocking_func(const void * data);

int
main(const int argc, const char * argv[])
{

    unsigned int     port     = 8080;
    const char       *err_msg = NULL;
    selector_status  ss       = SELECTOR_SUCCESS;
    fd_selector      selector = NULL;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server < 0) {
        err_msg = "unable to create socket";
        /** exit with error */
        return 1;
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        err_msg = "unable to bind socket";
        /** exit with error */
        return 1;
    }

    if (listen(server, 20) < 0) {
        err_msg = "unable to listen";
        /** exit with error */
        return 1;
    }

    /** Sets non-blocking io on server */

    if(selector_fd_set_nio(server) == -1) {
        err_msg = "getting server socket flags";
        /** exit with error */
        return 1;
    }

    const struct selector_init conf = {
            .signal = SIGALRM,
            .select_timeout = {
                    .tv_sec  = 10,
                    .tv_nsec = 0,
            },
    };

    if(selector_init(&conf) != 0) {
        err_msg = "initializing selector";
        /** exit with error */
        return 1;
    }

    selector = selector_new(1024);
    if(selector == NULL) {
        err_msg = "unable to create selector";
        /** exit with error */
        return 1;
    }

    const struct fd_handler example_handler = {
            .handle_read       = listen_read_handler,
            .handle_write      = NULL,
            .handle_close      = NULL, // nada que liberar
            .handle_block      = NULL,
    };

    ss = selector_register(selector, server, &example_handler, OP_READ, NULL);

    if(ss != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        /** exit with error */
        return 1;
    }

    for(;;) {
        err_msg = NULL;
        ss = selector_select(selector);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            /** exit with error */
            return 1;
        }
    }

}

void
listen_read_handler(struct selector_key *key)
{
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len = sizeof(client_addr);

    const int client = accept(key->fd, (struct sockaddr*) &client_addr,
                              &client_addr_len);

    if(client == -1) {
        /** return with error */
        close(client);
        return;
    }

    printf("Client accepted, closing connection...\n");

    close(client);

}
