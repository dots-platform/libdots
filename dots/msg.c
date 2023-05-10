#include "dots/msg.h"
#include <arpa/inet.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "dots/env.h"
#include "dots/err.h"
#include "dots/request.h"
#include "dots/internal/control_msg.h"
#include "dots/internal/env.h"
#include "dots/internal/util.h"

#define INITIAL_RANK_TAG_SOCKETS_CAP 8

struct rank_tag_socket {
    size_t rank;
    int tag;
    int socket;
};

int dots_msg_send(const dots_request_t *req, const void *buf_, size_t len,
        size_t recipient, int tag) {
    const unsigned char *buf = buf_;
    int ret;

    if (recipient >= dots_world_size || recipient == dots_world_rank) {
        ret = DOTS_ERR_INVALID;
        goto exit;
    }

    /* Send a MSG_SEND control message. */
    struct control_msg msg = {
        .data = {
            .msg_send = {
                .recipient = htonl(recipient),
                .tag = htonl(tag),
            },
        },
    };
    ret = dots_send_control_msg(req, &msg, CONTROL_MSG_TYPE_MSG_SEND, buf, len);
    if (ret) {
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

int dots_msg_recv(const dots_request_t *req, void *buf_, size_t len,
        size_t sender, int tag, size_t *recv_len) {
    unsigned char *buf = buf_;
    int ret;

    if (sender >= dots_world_size || sender == dots_world_rank) {
        ret = DOTS_ERR_INVALID;
        goto exit;
    }

    size_t ignored_recv_len;
    if (!recv_len) {
        recv_len = &ignored_recv_len;
    }

    /* Construct a MSG_RECV control message. */
    struct control_msg msg = {
        .data = {
            .msg_recv = {
                .sender = htonl(sender),
                .tag = htonl(tag),
            },
        },
    };

    /* Send control message. */
    ret = dots_send_control_msg(req, &msg, CONTROL_MSG_TYPE_MSG_RECV, NULL, 0);
    if (ret) {
        goto exit;
    }
    uint64_t send_msg_id = dots_ntohll(msg.hdr.msg_id);

    /* Receive data. */
    void *recv_data;
    ret = dots_recv_control_msg(send_msg_id, &msg, &recv_data, recv_len);
    if (ret) {
        goto exit;
    }

    /* If the buffer is too small, return an error. */
    if (len < *recv_len) {
        ret = DOTS_ERR_MSG_RECV_BUF_TOO_SMALL;
        goto exit;
    }

    /* Copy received data into user buffer. */
    memcpy(buf, recv_data, *recv_len);

    ret = 0;

    free(recv_data);
exit:
    return ret;
}
