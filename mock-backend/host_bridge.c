#include "host_bridge.h"
#include "async_low_level.h"

#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define PORT 3000

static void serialize_pointer(uint8_t dest[sizeof(void *)], void *ptr) {
    uintptr_t iptr = (uintptr_t) ptr;
    for (int i=0; i<sizeof(void *); i++) {
        dest[sizeof(void *) - 1 - i] = (iptr >> (8 * i)) & 0xff;
    }
}

static void *deserialize_pointer(uint8_t src[sizeof(void *)]) {
    uintptr_t ret = 0;
    for (int i=0; i<sizeof(void *); i++) {
        ret |= ((uintptr_t) src[sizeof(void *) - 1 - i]) << (i * 8);
    }
    return (void *) ret;
}

// TODO make work with not just 8 byte pointers
// static void print_serialized_pointer(uint8_t ser[sizeof(void *)]) {
//     printf("%02x%02x%02x%02x%02x%02x%02x%02x\n", ser[0], ser[1], ser[2], ser[3], ser[4], ser[5], ser[6], ser[7]);
// }

static int client;

static void connect_to_client() {
    int server_fd;
    // int new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    // char buffer[1024] = { 0 };
    // char* hello = "Hello from server";
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Forcefully attaching socket to the port 8080
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
  
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((client
         = accept(server_fd, (struct sockaddr*)&address,
                  (socklen_t*)&addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    if (fcntl(client, F_SETFL, fcntl(client, F_GETFL, 0) | O_NONBLOCK)) {
        perror("fnctl error");
        exit(EXIT_FAILURE);
    }

    // valread = read(new_socket, buffer, 1024);
    // printf("%s\n", buffer);
    // send(new_socket, hello, strlen(hello), 0);
    // printf("Hello message sent\n");
  
    // // closing the connected socket
    // close(new_socket);

    // closing the listening socket
    if (shutdown(server_fd, SHUT_RDWR) < 0) {
        perror("shutdown");
        exit(EXIT_FAILURE);
    }
    puts("connected");
}

static int epollfd;

static void update_client_poll_type(uint32_t op) {
    struct epoll_event ev;
    ev.events = op;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, client, &ev) == -1) {
        perror("epoll_ctl: update client for poll type");
        exit(EXIT_FAILURE);
    }
}

static void setup_epoll() {
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &ev) == -1) {
        perror("epoll_ctl: register client for read polling");
        exit(EXIT_FAILURE);
    }
}

typedef struct d_l_list_node_t {
    struct d_l_list_node_t *forward;
    void *value;
    int len;
    struct d_l_list_node_t *backward;
} DLListNode;
static void dllist_push(DLListNode **back, void *value, int len) {
    DLListNode *new = malloc(sizeof(DLListNode));
    new->value = value;
    new->len = len;
    if (*back == NULL) {
        new->forward = NULL;
    } else {
        (*back)->backward = new;
        new->forward = *back;
    }
    new->backward = NULL;
    *back = new;
}
static void dllist_discard(DLListNode **front) {
    DLListNode *node_being_discarded = *front;
    *front = node_being_discarded->backward;
    if (node_being_discarded->backward)
        node_being_discarded->backward->forward = NULL;
    free(node_being_discarded);
}

typedef struct {
    int front_i;
    DLListNode *front;
    DLListNode *back;
} TxQueue;
static void star_push(TxQueue *self, uint8_t *value, int len) {
    uint8_t *new = malloc(len);
    memcpy(new, value, len);
    dllist_push(&self->back, new, len);
    if (self->front == NULL) {
        self->front = self->back;
        self->front_i = 0;
    }
}
static void star_discard(TxQueue *self) {
    free(self->front->value);
    dllist_discard(&self->front);
    if (self->front == NULL) {
        self->back = NULL;
    }
    self->front_i = 0;
}
static TxQueue tx_queue;
static bool is_polling_write = false;

static void enque(uint8_t *data, int len) {
    // if (len == sizeof(void *)) print_serialized_pointer(data);
    star_push(&tx_queue, data, len);
    if (!is_polling_write) {
        is_polling_write = true;
        update_client_poll_type(EPOLLIN | EPOLLOUT);
    }
}

static ListNode *writer_stack = NULL;
static ASYNC_DEFINE_FUNCTION(writer_task, void)
    writer_stack = async_get_task_stack();
    while (1) {
        ASYNC_DETACH();
        while (1) {
            assert(tx_queue.front != NULL);
            ssize_t send_result = send(
                client,
                tx_queue.front->value + tx_queue.front_i,
                tx_queue.front->len - tx_queue.front_i,
                0
            );
            if (send_result >= 0) {
                tx_queue.front_i += send_result;
                if (tx_queue.front_i == tx_queue.front->len) {
                    star_discard(&tx_queue);
                    if (tx_queue.front == NULL) {
                        is_polling_write = false;
                        update_client_poll_type(EPOLLIN);
                        break;
                    }
                }
            }
            else {
                assert((errno & EAGAIN) || (errno & EWOULDBLOCK));
                break;
            }
        }
    }
ASYNC_DEFINE_FUNCTION_END

static ListNode *reader_stack = NULL;
static ListNode *wake_me_up = NULL;

struct reader_task_sub_data {
    void *dest;
    int len;
    int i;
};
static struct reader_task_sub_data *make_reader_sub_data(void *dest, int len) {
    struct reader_task_sub_data *new = malloc(sizeof(struct reader_task_sub_data));
    new->dest = dest;
    new->len = len;
    new->i = 0;
    return new;
}
static ASYNC_DEFINE_FUNCTION(reader_task_sub, struct reader_task_sub_data)
    while (1) {
        ssize_t recv_result = recv(client, data->dest + data->i, data->len - data->i, 0);
        if (recv_result == 0) {
            perror("recv: client closed the connection");
            exit(EXIT_SUCCESS);
        }
        if (recv_result > 0) {
            data->i += recv_result;
            if (data->i == data->len) {
                break;
            }
        }
        else {
            assert((errno & EAGAIN) || (errno & EWOULDBLOCK));
            reader_stack = async_get_task_stack();
            ASYNC_DETACH();
        }
    }
    free(data);
ASYNC_DEFINE_FUNCTION_END

struct {
    uint8_t wakee_stack[sizeof(void *)];
    uint8_t wakee_ret[sizeof(void *)];
    uint8_t ret_len;
    uint8_t *local_ret;
} reader_task_data;
static ASYNC_DEFINE_FUNCTION(reader_task, void)
    while (1) {
        ASYNC_AWAIT(reader_task_sub, make_reader_sub_data(reader_task_data.wakee_stack, sizeof(void *)));
        ASYNC_AWAIT(reader_task_sub, make_reader_sub_data(reader_task_data.wakee_ret, sizeof(void *)));
        ASYNC_AWAIT(reader_task_sub, make_reader_sub_data(&reader_task_data.ret_len, 1));
        if (reader_task_data.ret_len) {
            reader_task_data.local_ret = malloc(reader_task_data.ret_len);
            ASYNC_AWAIT(reader_task_sub, make_reader_sub_data(reader_task_data.local_ret, reader_task_data.ret_len));
        } else {
            reader_task_data.local_ret = NULL;
        }
        *((uint8_t **) deserialize_pointer(reader_task_data.wakee_ret)) = reader_task_data.local_ret;
        wake_me_up = deserialize_pointer(reader_task_data.wakee_stack);
        reader_stack = async_get_task_stack();
        ASYNC_DETACH();
    }
ASYNC_DEFINE_FUNCTION_END

static void setup_reader_and_writer_stacks() {
    // writer
    tx_queue.front_i = 0;
    tx_queue.front = NULL;
    tx_queue.back = NULL;
    async_spawn(writer_task, NULL);

    // reader
    async_spawn(reader_task, NULL);
}

ListNode *host_wait_for_ms_or_event(int wait_time) {
    if (wake_me_up != NULL) {
        ListNode *ret = wake_me_up;
        wake_me_up = NULL;
        return ret;
    }
    struct epoll_event events[2];
    int epoll_wait_return = epoll_wait(epollfd, events, 2, wait_time);
    if (epoll_wait_return < 0) {
        perror("epoll_wait: error waiting for event or timeout");
        exit(EXIT_FAILURE);
    }
    if (epoll_wait_return > 1) {
        perror("epoll_wait: 2 events received when not expected");
        exit(EXIT_FAILURE);
    }
    if (epoll_wait_return == 0) {
        return NULL;
    }
    if (events[0].events & EPOLLOUT) {
        assert(writer_stack);
        return writer_stack;
    }
    assert(events[0].events == EPOLLIN);
    assert(reader_stack);
    return reader_stack;
}

ASYNC_DEFINE_FUNCTION(host_transmit, host_transmit_data)
    enque(&data->code, 1);
    enque(data->bytes, data->len);
    uint8_t ptr_size = sizeof(void *);
    enque(&ptr_size, 1);

    void *ts = async_get_task_stack();
    // printf("%p\n", ts);
    uint8_t ser_ptr[sizeof(void *)];
    serialize_pointer(ser_ptr, ts);
    enque(ser_ptr, sizeof(void *));

    // printf("%p\n", &data->ret);
    serialize_pointer(ser_ptr, &data->ret);
    enque(ser_ptr, sizeof(void *));
    ASYNC_DETACH();
ASYNC_DEFINE_FUNCTION_END

void host_init() {
    connect_to_client();
    setup_epoll();
    setup_reader_and_writer_stacks();
}
