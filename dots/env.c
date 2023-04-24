#include "dots/env.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "dots/err.h"
#include "dots/internal/defs.h"

struct env_input {
    uint32_t world_rank;
    uint32_t world_size;
    uint32_t input_files_offset;
    uint32_t input_files_count;
    uint32_t output_files_offset;
    uint32_t output_files_count;
    uint32_t func_name_offset;
    uint32_t control_socket;
    uint32_t args_offset;
    uint32_t args_count;
    unsigned char unused[88];
    unsigned char data[];
} PACKED;

struct env_input_iovec {
    uint32_t offset;
    uint32_t length;
} PACKED;

static_assert(sizeof(struct env_input) == 128, "Size of environment input is not 128 bytes!");

static struct env_input *env_input;

size_t dots_world_rank;
size_t dots_world_size;
int32_t *dots_in_fds;
size_t dots_in_fds_len;
int32_t *dots_out_fds;
size_t dots_out_fds_len;
char *dots_func_name;
dots_env_arg_t *dots_args;
size_t dots_args_len;
int dots_control_socket;

int dots_env_init(void) {
    int ret;

    /* Read environment length. */
    uint32_t env_len;
    if (fread(&env_len, 1, sizeof(env_len), stdin) != 4) {
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }
    env_len = ntohl(env_len);

    /* Allocate environment buffer. */
    env_input = malloc(env_len);
    if (!env_input) {
        ret = DOTS_ERR_LIBC;
        goto exit;
    }

    /* Read rest of the environment. */
    if (fread(env_input, 1, env_len, stdin) != env_len) {
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }

    /* Set environment vars. */
    dots_world_rank = ntohl(env_input->world_rank);
    dots_world_size = ntohl(env_input->world_size);
    dots_in_fds =
        (int32_t *) ((unsigned char *) env_input
                + ntohl(env_input->input_files_offset));
    dots_in_fds_len = ntohl(env_input->input_files_count);
    for (size_t i = 0; i < dots_in_fds_len; i++) {
        dots_in_fds[i] = ntohl(dots_in_fds[i]);
    }
    dots_out_fds =
        (int32_t *) ((unsigned char *) env_input
                + ntohl(env_input->output_files_offset));
    dots_out_fds_len = ntohl(env_input->output_files_count);
    for (size_t i = 0; i < dots_out_fds_len; i++) {
        dots_out_fds[i] = ntohl(dots_out_fds[i]);
    }
    dots_func_name =
        (char *) ((unsigned char *) env_input
                + ntohl(env_input->func_name_offset));
    dots_control_socket = ntohl(env_input->control_socket);

    /* Set args. */
    dots_args_len = ntohl(env_input->args_count);
    dots_args = malloc(dots_args_len * sizeof(*dots_args));
    if (dots_args_len > 0 && !dots_args) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_env_input;
    }
    struct env_input_iovec *arg_iovec =
        (struct env_input_iovec *) ((unsigned char *) env_input
                + ntohl(env_input->args_offset));
    for (size_t i = 0; i < dots_args_len; i++) {
        dots_args[i].ptr =
            (unsigned char *) env_input + ntohl(arg_iovec[i].offset);
        dots_args[i].length = ntohl(arg_iovec[i].length);
    }

    return 0;

exit_free_env_input:
    free(env_input);
exit:
    return ret;
}

void dots_env_finalize(void) {
    free(env_input);
    free(dots_args);
}
