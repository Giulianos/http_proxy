#include "management.h"



void management_read(struct selector_key *key);
void management_write(struct selector_key *key);
void management_close(struct selector_key *key);

//MAGIA
int get_message(int server_fd, struct sockaddr_in* sender_addr)
{
    char payload[1024];
    int buffer_len = sizeof(payload) - 1;
    memset(&payload, 0, sizeof(payload));

    struct iovec io_buf;
    memset(&payload, 0, sizeof(payload));
    io_buf.iov_base = payload;
    io_buf.iov_len = buffer_len;

    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_iov = &io_buf;
    msg.msg_iovlen = 1;
    msg.msg_name = sender_addr;
    msg.msg_namelen = sizeof(struct sockaddr_in);

    while(1) {
        int recv_size = 0;
        if((recv_size = recvmsg(server_fd, &msg, 0)) == -1) {
            printf("recvmsg() error\n");
            return 1;
        }

        if(msg.msg_flags & MSG_EOR) {
            printf("%s\n", payload);
            break;
        }
        else {
            printf("%s", payload); //if EOR flag is not set, the buffer is not big enough for the whole message
        }
    }

    return 0;
}

int send_reply(int server_fd, struct sockaddr_in* dest_addr)

{
    char buf[8];
    memset(buf, 0, sizeof(buf));
    strncpy(buf, "OK", sizeof(buf)-1);

    struct iovec io_buf;
    io_buf.iov_base = buf;
    io_buf.iov_len = sizeof(buf);

    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_iov = &io_buf;
    msg.msg_iovlen = 1;
    msg.msg_name = dest_addr;
    msg.msg_namelen = sizeof(struct sockaddr_in);

    if(sendmsg(server_fd, &msg, 0) == -1) {
        printf("sendmsg() error %d\n");
        return 1;
    }

    return 0;
}

//FIN MAGIA


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
create_management_socket(in_addr_t address,in_port_t port){
    int management_socket;
//    int sock_opt = true;
    struct sockaddr_in serv_addr; ;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = address;
    serv_addr.sin_port = port;

    // create a master socket
    if ((management_socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) == 0) {
        return -1;
    }
    if(setsockopt(management_socket,SOL_SOCKET,SO_REUSEADDR,&(int){ 1 }, sizeof(int))){
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

void
management_read(struct selector_key *key){
    struct management *data = ATTACHMENT(key);
    get_message(data->client_fd,&data->addr_buf);


    if (selector_set_interest(key->s, key->fd, OP_WRITE) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, data->client_fd);
    }



}


void
management_write(struct selector_key *key){
    struct management * data = ATTACHMENT(key);
    send_reply(data->client_fd,&data->addr_buf);

    if (selector_set_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, data->client_fd);
    }


}

void
management_close(struct selector_key *key){
    struct management * data = ATTACHMENT(key);
    management_destroy(data);
}





