#include "dots/msg.h"
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "dots/control.h"
#include "dots/env.h"
#include "dots/err.h"

#define INITIAL_RANK_TAG_SOCKETS_CAP 8

struct rank_tag_socket {
    size_t rank;
    int tag;
    int socket;
};

/* A sorted list of tag -> socket. */
static struct rank_tag_socket *rank_tag_sockets;
static size_t rank_tag_sockets_len;
static size_t rank_tag_sockets_cap;

/* Comparator for rank_tag_socket. */
static int comp_rank_tag_socket(size_t rank, int tag,
        const struct rank_tag_socket *rank_tag_socket) {
    if (rank_tag_socket->rank != rank) {
        return (rank > rank_tag_socket->rank) - (rank < rank_tag_socket->rank);
    } else {
        return tag - rank_tag_socket->tag;
    }
}

/* Get or create a tag_socket entry in tag_sockets for the given tag. Use a
 * binary search to get the position to insert or return. */
static int get_or_create_rank_tag_socket(size_t rank, int tag, int *socket) {
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    int ret;

    if (pthread_mutex_lock(&lock)) {
        ret = DOTS_ERR_LIBC;
        goto exit;
    }

    /* Binary search to search for the rank_tag_socket. */
    size_t left = 0;
    size_t right = rank_tag_sockets_len;
    while (left < right) {
        size_t mid = (left + right) / 2;
        int comp = comp_rank_tag_socket(rank, tag, &rank_tag_sockets[mid]);
        if (comp == 0) {
            left = mid;
            break;
        } else if (comp < 0) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    /* Insert if needed. */
    if (rank_tag_sockets_len == 0
            || rank_tag_sockets[left].rank != rank
            || rank_tag_sockets[left].tag != tag) {
        /* Open a new socket. */
        ret = dots_open_socket(rank);
        if (ret < 0) {
            goto exit_unlock;
        }
        int new_socket = ret;

        /* Extend array if needed. */
        if (rank_tag_sockets_len == rank_tag_sockets_cap) {
            size_t new_cap =
                rank_tag_sockets_cap > 0
                    ? rank_tag_sockets_cap * 2
                    : INITIAL_RANK_TAG_SOCKETS_CAP;
            struct rank_tag_socket *new_rank_tag_sockets =
                realloc(rank_tag_sockets, new_cap * sizeof(*rank_tag_sockets));
            if (!new_rank_tag_sockets) {
                ret = DOTS_ERR_LIBC;
                goto exit_unlock;
            }
            rank_tag_sockets = new_rank_tag_sockets;
            rank_tag_sockets_cap = new_cap;
        }

        /* Insert into sorted position in array. */
        memmove(rank_tag_sockets + left + 1, rank_tag_sockets + left,
                (rank_tag_sockets_len - left) * sizeof(*rank_tag_sockets));
        rank_tag_sockets[left].rank = rank;
        rank_tag_sockets[left].tag = tag;
        rank_tag_sockets[left].socket = new_socket;
        rank_tag_sockets_len++;
    }

    /* Return socket. */
    *socket = rank_tag_sockets[left].socket;

    ret = 0;

exit_unlock:
    pthread_mutex_unlock(&lock);
exit:
    return ret;
}

int dots_msg_send(const void *buf_, size_t len, size_t recipient, int tag) {
    const unsigned char *buf = buf_;
    int ret;

    if (recipient >= dots_world_size || recipient == dots_world_rank) {
        ret = DOTS_ERR_INVALID;
        goto exit;
    }

    int socket;
    if (tag == 0) {
        socket = dots_comm_sockets[recipient];
    } else {
        ret = get_or_create_rank_tag_socket(recipient, tag, &socket);
        if (ret) {
            goto exit;
        }
    }

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

int dots_msg_recv(void *buf_, size_t len, size_t sender, int tag,
        size_t *recv_len) {
    unsigned char *buf = buf_;
    int ret;

    if (sender >= dots_world_size || sender == dots_world_rank) {
        ret = DOTS_ERR_INVALID;
        goto exit;
    }

    int socket;
    if (tag == 0) {
        socket = dots_comm_sockets[sender];
    } else {
        ret = get_or_create_rank_tag_socket(sender, tag, &socket);
        if (ret) {
            goto exit;
        }
    }

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
