#include "dots/env.h"
#include "dots/internal/env.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "dots/err.h"
#include "dots/internal/defs.h"

struct env_input {
    uint32_t control_socket;
    uint32_t world_rank;
    uint32_t world_size;
    unsigned char unused[116];
} PACKED;

static_assert(sizeof(struct env_input) == 128,
        "Size of environment input is not 128 bytes!");

int dots_control_socket;
size_t dots_world_rank;
size_t dots_world_size;

int dots_init(void) {
    int ret;

    /* Read environment input. */
    struct env_input input;
    int bytes_read = fread(&input, 1, sizeof(input), stdin);
    if (bytes_read < (int) sizeof(input)) {
        if (ferror(stdin)) {
            ret = DOTS_ERR_LIBC;
            goto exit;
        }
        ret = DOTS_ERR_INTERFACE;
        goto exit;
    }

    dots_control_socket = ntohl(input.control_socket);
    dots_world_rank = ntohl(input.world_rank);
    dots_world_size = ntohl(input.world_size);

    ret = 0;

exit:
    return ret;
}

void dots_finalize(void) {}

size_t dots_get_world_rank(void) {
    return dots_world_rank;
}

size_t dots_get_world_size(void) {
    return dots_world_size;
}
