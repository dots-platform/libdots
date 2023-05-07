#ifndef DOTS_INTERNAL_CONTROL_MSG_H
#define DOTS_INTERNAL_CONTROL_MSG_H

#include <stddef.h>
#include <stdint.h>
#include "dots/request.h"
#include "dots/internal/defs.h"

#define CONTROL_MSG_SIZE 64

/* #define CONTROL_MSG_TYPE_REQUEST_SOCKET 1u */ /* Deprecated. */
#define CONTROL_MSG_TYPE_MSG_SEND 2u
#define CONTROL_MSG_TYPE_MSG_RECV 3u
#define CONTROL_MSG_TYPE_MSG_RECV_RESP 4u
#define CONTROL_MSG_TYPE_OUTPUT 5u
#define CONTROL_MSG_TYPE_REQ_ACCEPT 6u
#define CONTROL_MSG_TYPE_REQ_ACCEPT_RESP 7u
#define CONTROL_MSG_TYPE_REQ_FINISH 8u

struct control_msg_hdr {
    uint16_t type;
    uint16_t unused0;
    uint32_t payload_len;
    unsigned char request_id[16];
    unsigned char unused1[8];
} PACKED;

_Static_assert(sizeof(struct control_msg_hdr) == 32,
        "Control message header size is not 32 bytes!");

struct control_msg_request_socket {
    uint32_t other_rank;
};

struct control_msg_msg_send {
    uint32_t recipient;
    uint32_t tag;
};

struct control_msg_msg_recv {
    uint32_t sender;
    uint32_t tag;
};

struct control_msg_msg_recv_resp {
    unsigned char unused;
};

struct control_msg_output {
    unsigned char unused;
};

struct control_msg_req_accept {
    unsigned char unused;
};

struct control_msg_req_accept_resp {
    unsigned char unused;
};

struct control_msg_req_finish {
    unsigned char unused;
};

struct control_msg {
    struct control_msg_hdr hdr;
    union {
        struct control_msg_request_socket request_socket;
        struct control_msg_msg_send msg_send;
        struct control_msg_msg_recv msg_recv;
        struct control_msg_msg_recv_resp msg_recv_resp;
        struct control_msg_output output;
        struct control_msg_req_accept req_accept;
        struct control_msg_req_accept_resp req_accept_resp;
        struct control_msg_req_finish req_finish;
        unsigned char bytes[32];
    } PACKED data;
} PACKED;

_Static_assert(sizeof(struct control_msg) == CONTROL_MSG_SIZE,
        "Control message size is not 64 bytes!");

int dots_send_control_msg(const dots_request_t *req, struct control_msg *msg,
        uint16_t type, const void *payload, size_t payload_len);
int dots_recv_control_msg(const dots_request_t *req, struct control_msg *msg,
        uint16_t type, void **payload, size_t *payload_len);

#endif
