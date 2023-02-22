#include "dots/control.h"
#include <arpa/inet.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include "dots/env.h"
#include "dots/err.h"

/* Send a message to the control socket. Messages are always a 4-byte length
 * followed by the message payload. */
static int send_control_msg(const void *buf_, size_t len) {
    // TODO This is probably something that would want locking.
    const unsigned char *buf = buf_;
    int ret;

    /* Ensure length fits within 4 bytes. */
    if (len > UINT32_MAX) {
        ret = DOTS_ERR_INTERNAL;
        goto exit;
    }
    uint32_t len_bytes = htonl(len);

    /* Send length. */
    size_t bytes_to_send = sizeof(len_bytes);
    size_t sent_so_far = 0;
    while (sent_so_far < bytes_to_send) {
        int bytes_sent =
            send(dots_control_socket,
                    (const unsigned char *) &len_bytes + sent_so_far,
                    sizeof(len_bytes) - sent_so_far, 0);
        if (bytes_sent < 0) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }
        sent_so_far += bytes_sent;
    }

    /* Send message. */
    bytes_to_send = len;
    sent_so_far = 0;
    while (sent_so_far < len) {
        int bytes_sent =
            send(dots_control_socket, buf + sent_so_far, len - sent_so_far, 0);
        if (bytes_sent < 0) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }
        sent_so_far += bytes_sent;
    }

    ret = 0;

exit:
    return ret;
}

static int recvfd(int control_sock) {
    /* Prepare to receive a one-byte message with a single socket as ancillary
     * data. */
    char iobuf[1];
    char fd_buf[CMSG_SPACE(sizeof(int))];
    struct iovec iov = {
        .iov_base = iobuf,
        .iov_len = sizeof(iobuf),
    };
    struct msghdr msg = {
        .msg_name = NULL,
        .msg_namelen = 0,
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = fd_buf,
        .msg_controllen = sizeof(fd_buf),
        .msg_flags = 0,
    };
    int ret;

    /* Receive from the control socket. */
    int bytes_recvd = recvmsg(control_sock, &msg, 0);
    if (bytes_recvd < 0) {
        ret = DOTS_ERR_LIBC;
        goto exit;
    }
    if (bytes_recvd == 0) {
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }

    /* Check that we have a new socket in the header. */
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg
            || cmsg->cmsg_len != CMSG_LEN(sizeof(int))
            || cmsg->cmsg_level != SOL_SOCKET
            || cmsg->cmsg_type != SCM_RIGHTS) {
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }

    /* Parse and return the socket. */
    ret = *((int *) CMSG_DATA(cmsg));
    if (ret < 0) {
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }

exit:
    return ret;
}

int dots_open_socket(size_t other_rank) {
    int ret;

    /* The DoTS server expects the arguments to be passed in the same order, so
     * choose the lower rank as the first rank. */
    size_t first_rank;
    size_t second_rank;
    if (dots_world_rank < other_rank) {
        first_rank = dots_world_rank;
        second_rank = other_rank;
    } else {
        first_rank = other_rank;
        second_rank = dots_world_rank;
    }

    /* Generate the message. */
    char cmsg[128];
    size_t cmsg_len =
        snprintf(cmsg, sizeof(cmsg), "REQUEST_SOCKET %zu %zu", first_rank,
                second_rank);
    if (cmsg_len >= 128) {
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }

    /* Send the message to the control socket. */
    ret = send_control_msg(cmsg, cmsg_len);
    if (ret) {
        goto exit;
    }

    /* Receive a file descriptor from the control socket. */
    ret = recvfd(dots_control_socket);

exit:
    return ret;
}
