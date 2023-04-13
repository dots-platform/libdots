#include "dots/control.h"
#include <arpa/inet.h>
#include <stddef.h>
#include <sys/socket.h>
#include "dots/env.h"
#include "dots/err.h"
#include "dots/internal/control_msg.h"

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
    ret = dots_send_control_msg(&msg, CONTROL_MSG_TYPE_REQUEST_SOCKET, NULL, 0);
    if (ret) {
        goto exit;
    }

    /* Receive a file descriptor from the control socket. */
    ret = recvfd(dots_control_socket);
    if (ret) {
        goto exit;
    }

exit:
    return ret;
}
