#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>


#include <selector/selector.h>
#include <client/client.h>
#include <logger/logger.h>
#include <netinet/sctp.h>
#include <msg_queue/msg_queue.h>
#include <admin_handlers/admin_handlers.h>
#include <config/config.h>
#include <argument/argument.h>
#include <strings.h>
#include "client/remote_handlers.h"
#include "client/client_private.h"

#define SERVPORT 9090
#define LISTENQ 10

void
listen_read_handler(struct selector_key *key);

struct log l;

int
main(const int argc, const char ** argv)
{

    const char       *err_msg = NULL;
    selector_status  ss       = SELECTOR_SUCCESS;
    fd_selector      selector = NULL;
    int              port     = 8080;

    /** sctp server variables */
    int admin_socket, msg_flags;
    int return_value;
    struct sockaddr_in serveraddr, cliaddr;
    struct sctp_sndrcvinfo sri;
    struct sctp_event_subscribe evnts;

    /** config init */
    initialize_configurations("text/plain", "cat");
    config_create("proxy_port", "8080");
    config_create("mgmt_port", "9090");

    argument_get(argc,argv);

    /** proxy socket */
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family      = AF_INET6;
    addr.sin6_addr = in6addr_any;
//    addr.sin6_port        = htons(port);
    addr.sin6_port        = htons((uint16_t) atoi(config_get("proxy_port")));

    /** admin_socket initialization */

    admin_socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if(admin_socket < 0) {
        err_msg = "creating socket";
        printf("%s\n",err_msg);
        return 1;
    }

    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVPORT);
//    serveraddr.sin_port=htons((uint16_t) atoi(config_get("mgmt_port")));

    return_value = bind(admin_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(return_value < 0) {
        err_msg = "binding socket";
        printf("%s\n",err_msg);
        return 1;
    }

    bzero(&evnts, sizeof(evnts));
    evnts.sctp_data_io_event = 1;
    return_value = setsockopt(admin_socket, IPPROTO_SCTP, SCTP_EVENTS, &evnts, sizeof(evnts));
    if(return_value < 0) {
        err_msg = "setting socket's options";
        printf("%s\n",err_msg);
        return 1;
    }

    return_value = listen(admin_socket, LISTENQ);
    if(return_value < 0) {
        err_msg = "listening socket";
        printf("%s\n",err_msg);
        return 1;
    }

    const int server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(server < 0) {
//        err_msg = "unable to create socket";
        /** exit with error */
        return 1;
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
//        err_msg = "unable to bind socket";
        /** exit with error */
        return 1;
    }

    if (listen(server, 20) < 0) {
//        err_msg = "unable to listen";
        /** exit with error */
        return 1;
    }

    /** Sets non-blocking io on server */

    if(selector_fd_set_nio(server) == -1) {
//        err_msg = "getting server socket flags";
        /** exit with error */
        return 1;
    }

    if(selector_fd_set_nio(admin_socket) == -1) {
        err_msg = "getting server socket flags";
        printf("%s\n",err_msg);
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
//        err_msg = "initializing selector";
        /** exit with error */
        return 1;
    }

    selector = selector_new(1024);
    if(selector == NULL) {
//        err_msg = "unable to create selector";
        /** exit with error */
        return 1;
    }

    const struct fd_handler example_handler = {
            .handle_read       = listen_read_handler,
            .handle_write      = NULL,
            .handle_close      = NULL, // nada que liberar
            .handle_block      = NULL,
    };

    log_start(&l,selector);
    fd_handler logging_handlers = {
            .handle_read = NULL,
            .handle_write = log_write,
            .handle_close = log_close
    };

    //ss = selector_register(selector,l.writefd,&logging_handlers,OP_WRITE,&l);
    if(ss != SELECTOR_SUCCESS) {
//        err_msg = "registering fd";
        /** exit with error */
        return 1;
    }

    ss = selector_register(selector, server, &example_handler, OP_READ, NULL);
    if(ss != SELECTOR_SUCCESS) {
//        err_msg = "registering fd";
        /** exit with error */
        return 1;
    }

    const struct fd_handler admin_handler = {
        .handle_read       = admin_read_handler,
        .handle_write      = admin_write_handler,
        .handle_close      = NULL,
        .handle_block      = NULL,
    };

    bzero(&sri, sizeof(sri));
    sri.sinfo_stream = 0;
    struct addr_data admin_data = {
        .addr       = (struct sockaddr *)&cliaddr,
        .len        = sizeof(cliaddr),
        .sri        = sri,
        .msg_flags  = msg_flags=0,
    };

    ss = selector_register(selector, admin_socket, &admin_handler, OP_READ, &admin_data);

    if(ss != SELECTOR_SUCCESS) {
        err_msg = "registering admin_socket";
        printf("%s\n", err_msg);
        return 1;
    }

    q_init(admin_socket, selector);

    for(;;) {
        if(q_is_empty()) {
            selector_set_interest(selector, admin_socket, OP_READ);
        }
        ss = selector_select(selector);
        if(!q_is_empty()) {
            selector_set_interest(selector, admin_socket, OP_READ | OP_WRITE);
        }
        if(ss != SELECTOR_SUCCESS) {
//            err_msg = "serving";
            /** exit with error */
            return 2;
        }
    }

}

fd_handler client_handlers = {
    .handle_read = client_read,
    .handle_write = client_write,
    .handle_close = client_close,
    .handle_block = client_block,
};

void
listen_read_handler(struct selector_key *key)
{
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len = sizeof(client_addr);

    const int client_socket = accept(key->fd, (struct sockaddr*) &client_addr,
                              &client_addr_len);

    if(client_socket == -1) {
        /** return with error */
        close(client_socket);
        return;
    }

    /** Create a client and add it to the selector */
    struct client_config config = {
        .selector = key->s,
        .fd = client_socket,
    };

    client_t client = client_new(&config);
    client->log=&l;
    selector_fd_set_nio(client_socket);
    selector_register(key->s, client_socket, &client_handlers, OP_READ, client);

}

int
initialize_configurations(const char * media_types, const char * cmd)
{
  if(media_types != NULL && cmd != NULL) {
    config_create("media_types", media_types);
    config_create("cmd", cmd);
  } else if(cmd != NULL) {
    config_create("media_types", "");
    config_create("cmd", cmd);
  } else if(media_types != NULL) {
    return -1;
  }
  config_create("buffers_size", "8192");
  return 1;
}