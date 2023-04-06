#ifndef DOTS_INTERNAL_CONTROL_MSG_H
#define DOTS_INTERNAL_CONTROL_MSG_H

#include <stdint.h>
#include "dots/internal/defs.h"

#define CONTROL_MSG_SIZE 64

#define CONTROL_MSG_TYPE_REQUEST_SOCKET 1u

struct control_msg_hdr {
    uint16_t type;
    uint16_t unused0;
    uint32_t payload_len;
    unsigned char unused1[24];
} PACKED;

_Static_assert(sizeof(struct control_msg_hdr) == 32,
        "Control message header size is not 32 bytes!");

struct control_msg_request_socket {
    uint32_t other_rank;
};

struct control_msg {
    struct control_msg_hdr hdr;
    union {
        struct control_msg_request_socket request_socket;
        unsigned char bytes[32];
    } PACKED data;
    unsigned char payload[];
} PACKED;

_Static_assert(sizeof(struct control_msg) == CONTROL_MSG_SIZE,
        "Control message size is not 64 bytes!");

#endif
