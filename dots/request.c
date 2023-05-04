#include "dots/request.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dots/err.h"
#include "dots/internal/control_msg.h"
#include "dots/internal/defs.h"

struct request_input {
    unsigned char id[16];
    uint32_t input_files_offset;
    uint32_t input_files_count;
    uint32_t output_files_offset;
    uint32_t output_files_count;
    uint32_t func_name_offset;
    uint32_t args_offset;
    uint32_t args_count;
    unsigned char unused[84];
    unsigned char data[];
} PACKED;

struct request_input_iovec {
    uint32_t offset;
    uint32_t length;
} PACKED;

static_assert(sizeof(struct request_input) == 128,
        "Size of request input is not 128 bytes!");

int dots_request_accept(dots_request_t *req) {
    int ret;

    /* Construct REQ_ACCEPT control message. */
    struct control_msg msg;
    ret =
        dots_send_control_msg(NULL, &msg, CONTROL_MSG_TYPE_REQ_ACCEPT, NULL, 0);
    if (ret) {
        goto exit;
    }

    /* Receive a REQ_ACCEPT_RESP control message. */
    uint16_t resp_type;
    void *req_input_v;
    size_t req_input_len;
    ret =
        dots_recv_control_msg(NULL, &msg, &resp_type, &req_input_v,
                &req_input_len);
    if (ret) {
        goto exit;
    }
    struct request_input *req_input = req_input_v;

    /* Set request vars. */
    memcpy(req->id, req_input->id, sizeof(req->id));
    req->in_fds =
        (int32_t *) ((unsigned char *) req_input
                + ntohl(req_input->input_files_offset));
    req->in_fds_len = ntohl(req_input->input_files_count);
    for (size_t i = 0; i < req->in_fds_len; i++) {
        req->in_fds[i] = ntohl(req->in_fds[i]);
    }
    req->out_fds =
        (int32_t *) ((unsigned char *) req_input
                + ntohl(req_input->output_files_offset));
    req->out_fds_len = ntohl(req_input->output_files_count);
    for (size_t i = 0; i < req->out_fds_len; i++) {
        req->out_fds[i] = ntohl(req->out_fds[i]);
    }
    req->func_name =
        (char *) ((unsigned char *) req_input
                + ntohl(req_input->func_name_offset));

    /* Set args. */
    req->args_len = ntohl(req_input->args_count);
    req->args = malloc(req->args_len * sizeof(*req->args));
    if (req->args_len > 0 && !req->args) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_req_input;
    }
    struct request_input_iovec *arg_iovec =
        (struct request_input_iovec *) ((unsigned char *) req_input
                + ntohl(req_input->args_offset));
    for (size_t i = 0; i < req->args_len; i++) {
        req->args[i].ptr =
            (unsigned char *) req_input + ntohl(arg_iovec[i].offset);
        req->args[i].length = ntohl(arg_iovec[i].length);
    }

    req->input = req_input;

    return 0;

exit_free_req_input:
    free(req_input);
exit:
    return ret;
}

int dots_request_finish(dots_request_t *req) {
    int ret;

    /* Construct and send REQ_FINISH control message. */
    struct control_msg msg;
    ret =
        dots_send_control_msg(req, &msg, CONTROL_MSG_TYPE_REQ_FINISH, NULL, 0);
    if (ret) {
        goto exit;
    }

exit:
    return ret;
}

void dots_request_free(dots_request_t *req) {
    free(req->input);
    free(req->args);
}
