#include "dots/control.h"
#include <arpa/inet.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include "dots/env.h"
#include "dots/err.h"
#include "dots/internal/control_msg.h"

static int sendall(int fd, const void *buf_, size_t len) {
    const unsigned char *buf = buf_;
    int ret;

    while (len) {
        int bytes_sent = send(fd, buf, len, 0);
        if (bytes_sent < 0) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }
        buf += bytes_sent;
        len -= bytes_sent;
    }

    ret = 0;

exit:
    return ret;
}

/* Send a message to the control socket. Messages are always a 4-byte length
 * followed by the message payload. */
static int send_control_msg(struct control_msg *msg, uint16_t type,
        const void *payload, size_t payload_len) {
    // TODO This is probably something that would want locking.
    int ret;

    /* Set message header values. */
    msg->hdr.type = htons(type);

    /* Ensure length fits within 4 bytes. */
    if (payload_len > UINT32_MAX) {
        ret = DOTS_ERR_INTERNAL;
        goto exit;
    }
    msg->hdr.payload_len = htonl(payload_len);

    /* Send header. */
    ret = sendall(dots_control_socket, msg, sizeof(*msg));
    if (ret) {
        goto exit;
    }

    /* Send payload. */
    ret = sendall(dots_control_socket, payload, payload_len);
    if (ret) {
        goto exit;
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

    /* Generate the message. */
    struct control_msg msg;
    msg.data.request_socket.other_rank = htonl(other_rank);

    /* Send the message to the control socket. */
    ret = send_control_msg(&msg, CONTROL_MSG_TYPE_REQUEST_SOCKET, NULL, 0);
    if (ret) {
        goto exit;
    }

    /* Receive a file descriptor from the control socket. */
    ret = recvfd(dots_control_socket);

exit:
    return ret;
}
