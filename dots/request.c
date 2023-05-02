#include "dots/request.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dots/err.h"
#include "dots/internal/defs.h"

struct request_input {
    uint32_t world_rank;
    uint32_t world_size;
    uint32_t input_files_offset;
    uint32_t input_files_count;
    uint32_t output_files_offset;
    uint32_t output_files_count;
    uint32_t func_name_offset;
    uint32_t args_offset;
    uint32_t args_count;
    unsigned char unused[92];
    unsigned char data[];
} PACKED;

struct request_input_iovec {
    uint32_t offset;
    uint32_t length;
} PACKED;

static_assert(sizeof(struct request_input) == 128, "Size of environment input is not 128 bytes!");

int dots_request_accept(dots_request_t *req) {
    int ret;

    /* Read request length. */
    uint32_t req_len;
    if (fread(&req_len, 1, sizeof(req_len), stdin) != 4) {
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }
    req_len = ntohl(req_len);

    /* Allocate request input buffer. */
    struct request_input *req_input = malloc(req_len);
    if (!req_input) {
        ret = DOTS_ERR_LIBC;
        goto exit;
    }

    /* Read rest of the environment. */
    if (fread(req_input, 1, req_len, stdin) != req_len) {
        ret = DOTS_ERR_INTERFACE;
        goto exit_free_req_input;
    }

    /* Set request vars. */
    req->world_rank = ntohl(req_input->world_rank);
    req->world_size = ntohl(req_input->world_size);
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

    return 0;

exit_free_req_input:
    free(req_input);
exit:
    return ret;
}

void dots_req_finalize(dots_request_t *req) {
    free(req->input);
    free(req->args);
}