#include "dots/msg.h"
#include <stddef.h>
#include <sys/socket.h>
#include "dots/env.h"
#include "dots/err.h"

int dots_msg_send(const void *buf_, size_t len, int recipient) {
    const unsigned char *buf = buf_;
    int ret;

    if (recipient < 0 || (size_t) recipient >= dots_world_size) {
        ret = DOTS_ERR_INVALID;
        goto exit;
    }

    int socket = dots_comm_sockets[recipient];

    size_t sent_so_far = 0;
    while (sent_so_far < len) {
        int bytes_sent = send(socket, buf + sent_so_far, len - sent_so_far, 0);
        if (bytes_sent < 0) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }
        if (bytes_sent == 0) {
            ret = DOTS_ERR_INTERFACE;
            goto exit;
        }

        sent_so_far += bytes_sent;
    }

    ret = 0;

exit:
    return ret;
}

int dots_msg_recv(void *buf_, size_t len, int sender, size_t *recv_len) {
    unsigned char *buf = buf_;
    int ret;

    if (sender < 0 || (size_t) sender >= dots_world_size) {
        ret = DOTS_ERR_INVALID;
        goto exit;
    }

    int socket = dots_comm_sockets[sender];

    size_t recvd_so_far = 0;
    while (recvd_so_far < len) {
        int bytes_recvd =
            recv(socket, buf + recvd_so_far, len - recvd_so_far, 0);
        if (bytes_recvd < 0) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }
        if (bytes_recvd == 0) {
            ret = DOTS_ERR_INTERFACE;
            goto exit;
        }

        recvd_so_far += bytes_recvd;
    }

    /* This is always set to len for now. */
    if (recv_len) {
        *recv_len = len;
    }

    ret = 0;

exit:
    return ret;
}
