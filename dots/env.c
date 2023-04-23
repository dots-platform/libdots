#include "dots/env.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
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
    unsigned char unused[100];
    unsigned char data[];
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

    /* Open control socket. */
    // TODO Some more sane path handling here would be nice, or the DoTS server
    // could just use a socketpair rather than relying on a socket in /tmp.
    dots_control_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (dots_control_socket < 0) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_env_input;
    }
    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
    };
    if (snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/socket-%d",
                getpid())
            >= (int) sizeof(addr.sun_path)) {
        ret = DOTS_ERR_INTERNAL;
        goto exit_close_control_socket;
    }
    if (connect(dots_control_socket, (struct sockaddr *) &addr, sizeof(addr))) {
        ret = DOTS_ERR_LIBC;
        goto exit_close_control_socket;
    }

    return 0;

exit_close_control_socket:
    close(dots_control_socket);
exit_free_env_input:
    free(env_input);
exit:
    return ret;
}

void dots_env_finalize(void) {
    free(env_input);
    close(dots_control_socket);
}
