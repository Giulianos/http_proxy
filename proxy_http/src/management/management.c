#include "management.h"
#include <netinet/sctp.h>



void management_read(struct selector_key *key);
void management_write(struct selector_key *key);
void management_close(struct selector_key *key);

struct management *
management_new(const int client_fd){
    struct management *ret;
    ret = malloc(sizeof(*ret));
    if (ret == NULL){
        return ret;
    }

    ret->client_fd     = client_fd;
    buffer_init(&ret->buffer_write, N(ret->raw_buffer_write),ret->raw_buffer_write);
    buffer_init(&ret->buffer_read , N(ret->raw_buffer_read) ,ret->raw_buffer_read);
//    ret->status = ST_HELO;
//    ret->error  = PARSE_OK;
//    ret->argc = 0;
//    ret->cmd = NULL;
    return ret;
}

void
management_destroy(struct management * m){
    free(m); //TODO: si hay malloc liberarlos
}

int
create_management_socket(struct addrinfo *addr){
    int management_socket;
    int sock_opt = true;
    struct sockaddr_in serv_addr; ;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); //TODO sacar el hardcodeo

    // create a master socket
    if ((management_socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) == 0) {
        return -1;
    }


    if (bind(management_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        return -1;
    }

    if (listen(management_socket, 5) < 0) {
        return -1;
    }
    return management_socket;
}
void send_ok(struct management * data, const char * text){
    char * msg = malloc(strlen("+OK: ") + strlen(text) + 2);
    strcpy(msg, "+OK: ");
    strcat(msg, text);
    strcat(msg, "\n");
    send(data->client_fd, msg, strlen(msg), 0);
    free(msg);
}


void
management_read(struct selector_key *key){
    struct management *data = ATTACHMENT(key);
    if (selector_set_interest(key->s, key->fd, OP_WRITE) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, data->client_fd);
    }

}


void
management_write(struct selector_key *key){
    struct management * data = ATTACHMENT(key);
    if (selector_set_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, data->client_fd);
    }

}

void
management_close(struct selector_key *key){
    struct management * data = ATTACHMENT(key);
    management_destroy(data);
}





