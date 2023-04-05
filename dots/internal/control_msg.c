#include "dots/internal/control_msg.h"
#include <arpa/inet.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include "dots/env.h"
#include "dots/err.h"

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

/* Send a message to the control socket. */
int dots_send_control_msg(struct control_msg *msg, uint16_t type,
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
