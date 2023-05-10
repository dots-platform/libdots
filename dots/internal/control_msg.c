#include "dots/internal/control_msg.h"
#include <arpa/inet.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "dots/err.h"
#include "dots/request.h"
#include "dots/internal/env.h"
#include "dots/internal/util.h"

static_assert(sizeof(size_t) >= sizeof(uint32_t),
        "size_t must be able to hold at least a uint32_t");

static atomic_uint_fast64_t msg_counter;

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
int dots_send_control_msg(const dots_request_t *req, struct control_msg *msg,
        uint16_t type, const void *payload, size_t payload_len) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int ret;

    /* Set message header values. */
    msg->hdr.msg_id =
        atomic_fetch_add_explicit(&msg_counter, 1, memory_order_relaxed);
    msg->hdr.msg_id = dots_htonll(msg->hdr.msg_id);
    msg->hdr.type = htons(type);
    if (!req) {
        memset(msg->hdr.request_id, '\0', sizeof(msg->hdr.request_id));
    } else {
        memcpy(msg->hdr.request_id, req->id, sizeof(msg->hdr.request_id));
    }

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

struct recv_wait_list {
    uint64_t resp_msg_id;
    struct control_msg *msg;
    void **payload;
    size_t *payload_len;

    pthread_cond_t wake;
    bool is_fulfilled;

    struct recv_wait_list *next;
};

struct recv_msg_list {
    uint64_t resp_msg_id;
    struct control_msg msg;
    void *payload;
    size_t payload_len;

    struct recv_msg_list *next;
};

/* Receive a message from the control socket. */
int dots_recv_control_msg(uint64_t resp_msg_id, struct control_msg *msg,
        void **payload, size_t *payload_len) {
    static struct recv_wait_list *wait_list;
    static struct recv_msg_list *msg_list;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool receiver_exists;
    int ret;

    pthread_mutex_lock(&mutex);

    /* Search the stored message list for our result. If found, return. */
    struct recv_msg_list **prev_next = &msg_list;
    for (struct recv_msg_list *cur = msg_list; cur != NULL;
            prev_next = &cur->next, cur = cur->next) {
        if (cur->resp_msg_id == resp_msg_id) {
            *msg = cur->msg;
            *payload = cur->payload;
            *payload_len = cur->payload_len;
            *prev_next = cur->next;

            pthread_mutex_unlock(&mutex);
            ret = 0;
            goto exit;
        }
    }

    if (receiver_exists) {
        /* There is another reciever. Put ourselves on the wait list and wait
         * for someone to signal us. */
        struct recv_wait_list waiter = {
            .resp_msg_id = resp_msg_id,
            .msg = msg,
            .payload = payload,
            .payload_len = payload_len,
            .wake = PTHREAD_COND_INITIALIZER,
            .is_fulfilled = false,
            .next = wait_list,
        };
        wait_list = &waiter;
        pthread_cond_wait(&waiter.wake, &mutex);

        /* Now, someone has woken us up. If is_fulfilled, then we were woken
         * because someone else fulfilled our request, so we can safely return
         * now. */
        if (waiter.is_fulfilled) {
            ret = 0;
            pthread_mutex_unlock(&mutex);
            goto exit;
        } else {
            /* Else, we were woken up because the previous receiver is exiting. */
            assert(receiver_exists);
        }
    } else {
        /* There is no receiver, so we will be the receiver. */
        receiver_exists = true;
    }

    /* We only fall through to here as long as 1) our message was not in the
     * message list and 2) we need to be the receiver. */
    pthread_mutex_unlock(&mutex);

    bool found_our_message = false;
    while (!found_our_message) {
        /* Receive header. */
        ret = recvall(dots_control_socket, msg, sizeof(*msg));
        if (ret) {
            goto exit;
        }

        /* Parse header values. */
        uint64_t resp_resp_msg_id = dots_ntohll(msg->hdr.resp_msg_id);
        *payload_len = ntohl(msg->hdr.payload_len);

        /* Allocate space for payload. */
        *payload = malloc(*payload_len);
        if (!*payload) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }

        /* Receive payload. */
        ret = recvall(dots_control_socket, *payload, *payload_len);
        if (ret) {
            free(*payload);
            goto exit;
        }

        if (resp_resp_msg_id == resp_msg_id) {
            /* If we found our message, break out of the loop. */
            found_our_message = true;
        } else {
            /* Else, we found someone else's message. */
            pthread_mutex_lock(&mutex);

            /* First, see if a waiter for this message already exists. If so,
             * copy the message data directly to the waiter's return values. */
            struct recv_wait_list **prev_next = &wait_list;
            bool found_other_waiter = false;
            for (struct recv_wait_list *cur = wait_list; cur != NULL;
                    prev_next = &cur->next, cur = cur->next) {
                if (cur->resp_msg_id == resp_resp_msg_id) {
                    *cur->msg = *msg;
                    *cur->payload = *payload;
                    *cur->payload_len = *payload_len;
                    cur->is_fulfilled = true;
                    *prev_next = cur->next;
                    pthread_cond_signal(&cur->wake);

                    found_other_waiter = true;
                    break;
                }
            }

            /* If we didn't find another waiter for this, just dump it in the
             * recieve list. */
            if (!found_other_waiter) {
                struct recv_msg_list *stored_msg = malloc(sizeof(*stored_msg));
                if (!stored_msg) {
                    free(*payload);
                    pthread_mutex_unlock(&mutex);
                    ret = DOTS_ERR_LIBC;
                    goto exit;
                }
                *stored_msg = (struct recv_msg_list) {
                    .resp_msg_id = resp_resp_msg_id,
                    .msg = *msg,
                    .payload = *payload,
                    .payload_len = *payload_len,
                    .next = msg_list,
                };
                msg_list = stored_msg;
            }

            pthread_mutex_unlock(&mutex);

            /* Since we didn't find our message, we will loop again to try
             * receiving another message. */
        }
    }

    /* Now that we're out of the loop, signal another receiver if needed and
     * clear the receiver flag. */
    pthread_mutex_lock(&mutex);
    if (wait_list) {
        pthread_cond_signal(&wait_list->wake);
        wait_list = wait_list->next;
    } else {
        receiver_exists = false;
    }
    pthread_mutex_unlock(&mutex);

    ret = 0;

exit:
    return ret;
}
