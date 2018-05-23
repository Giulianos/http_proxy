#include "management.h"



void management_read(struct selector_key *key);
void management_write(struct selector_key *key);
void management_close(struct selector_key *key);

//MAGIA
int get_message(int server_fd, struct sockaddr_in6* sender_addr,struct management *data)
{
    size_t count;
    uint8_t * ptr = buffer_write_ptr(&data->buffer_read, &count);
    bool error = false;
    char * pass="passw0rd";

    char payload[1024];
    memset(&payload, 0, sizeof(payload));

    struct iovec io_buf;

    io_buf.iov_base = ptr;
    io_buf.iov_len = count;

    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_iov = &io_buf;
    msg.msg_iovlen = 1;
    msg.msg_name = sender_addr;

    msg.msg_namelen = sizeof(struct sockaddr_in6);

    unsigned int bytes=0;
    while(true) {
        ssize_t recv_size = 0;
        if((recv_size = recvmsg(server_fd, &msg, 0)) == -1) {
            printf("recvmsg() error\n");
            return 1;
        }else{
            buffer_write_adv(&data->buffer_read, recv_size);
            u_int8_t c;
            buffer *b=&data->buffer_read;
            while(buffer_can_read(b)) {
                c = buffer_read(b);

                printf("--%c\n", c); //TODO sacar blokea
                if(bytes==0)
                        data->message_type= (uint8_t) c;
                else if(bytes==1)
                    data->cookie= (uint8_t) c;
                bytes++;
            }
            if(msg.msg_flags & MSG_EOR) {
                break;
            }
        }


    }

    return 0;
}

int send_reply(int server_fd, struct sockaddr_in6* dest_addr, struct management *data)
{
    char buf[8];
    memset(buf, 0, sizeof(buf));
//    strncpy(buf, "OK", sizeof(buf)-1);
    buf[0]=data->message_type;
    buf[1]=data->cookie;

    struct iovec io_buf;
    io_buf.iov_base = buf;
    io_buf.iov_len = sizeof(buf);

    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_iov = &io_buf;
    msg.msg_iovlen = 1;
    msg.msg_name = dest_addr;
    msg.msg_namelen = sizeof(struct sockaddr_in6);

    if(sendmsg(server_fd, &msg, 0) == -1) {
        printf("sendmsg() error %d\n");
        return 1;
    }


    data->message_type=0;
    data->cookie=0;

    return 0;
}



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
    struct sockaddr_in6 serv_addr; ;

    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = port;

    // create a master socket
    if ((management_socket = socket(AF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP)) == 0) {
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
    get_message(data->client_fd,&data->addr_buf,data);


    if (selector_set_interest(key->s, key->fd, OP_WRITE) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, data->client_fd);
    }



}


void
management_write(struct selector_key *key){
    struct management * data = ATTACHMENT(key);
    send_reply(data->client_fd,&data->addr_buf,data);

    if (selector_set_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, data->client_fd);
    }


}

void
management_close(struct selector_key *key){
    struct management * data = ATTACHMENT(key);
    management_destroy(data);
}





