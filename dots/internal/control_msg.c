#include "dots/internal/control_msg.h"
#include <arpa/inet.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "dots/env.h"
#include "dots/err.h"
#include "dots/internal/env.h"

static_assert(sizeof(size_t) >= sizeof(uint32_t),
        "size_t must be able to hold at least a uint32_t");

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

static int recvall(int fd, void *buf_, size_t len) {
    unsigned char *buf = buf_;
    int ret;

    while (len) {
        int bytes_recvd = recv(fd, buf, len, 0);
        if (bytes_recvd < 0) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }
        buf += bytes_recvd;
        len -= bytes_recvd;
    }

    ret = 0;

exit:
    return ret;
}

/* Send a message to the control socket. */
int dots_send_control_msg(struct control_msg *msg, uint16_t type,
        const void *payload, size_t payload_len) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int ret;

    /* Set message header values. */
    msg->hdr.type = htons(type);

    /* Ensure length fits within 4 bytes. */
    if (payload_len > UINT32_MAX) {
        ret = DOTS_ERR_INTERNAL;
        goto exit;
    }
    msg->hdr.payload_len = htonl(payload_len);

    pthread_mutex_lock(&mutex);

    /* Send header. */
    ret = sendall(dots_control_socket, msg, sizeof(*msg));
    if (ret) {
        goto exit_unlock_mutex;
    }

    /* Send payload. */
    ret = sendall(dots_control_socket, payload, payload_len);
    if (ret) {
        goto exit_unlock_mutex;
    }

    ret = 0;

exit_unlock_mutex:
    pthread_mutex_unlock(&mutex);
exit:
    return ret;
}

/* Receive a message from the control socket. */
int dots_recv_control_msg(struct control_msg *msg, uint16_t *type,
        void **payload, size_t *payload_len) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int ret;

    pthread_mutex_lock(&mutex);

    /* Receive header. */
    ret = recvall(dots_control_socket, msg, sizeof(*msg));
    if (ret) {
        goto exit_unlock_mutex;
    }

    /* Parse header values. */
    *type = ntohs(msg->hdr.type);
    *payload_len = ntohl(msg->hdr.payload_len);

    /* Allocate space for payload. */
    *payload = malloc(*payload_len);
    if (!*payload) {
        ret = DOTS_ERR_LIBC;
        goto exit_unlock_mutex;
    }

    /* Receive payload. */
    ret = recvall(dots_control_socket, *payload, *payload_len);
    if (ret) {
        goto exit_free_payload;
    }

    ret = 0;

exit_free_payload:
    if (ret) {
        free(*payload);
    }
exit_unlock_mutex:
    pthread_mutex_unlock(&mutex);
    return ret;
}
