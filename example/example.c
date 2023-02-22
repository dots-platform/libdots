#include "dots.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    if (dots_env_init()) {
        abort();
    }

    /* Send bytes between sockets for a while for testing. */
    uint32_t bytes;
    for (size_t sender = 0; sender < dots_world_size; sender++) {
        if (dots_world_rank == sender) {
            bytes = htonl(sender);
            for (size_t recipient = 0; recipient < dots_world_size;
                    recipient++) {
                if (dots_msg_send(&bytes, sizeof(bytes), recipient)) {
                    abort();
                }
                if (dots_world_rank == recipient) {
                    if (dots_msg_recv(&bytes, sizeof(bytes), sender, NULL)) {
                        abort();
                    }
                    assert(bytes == htonl(sender));
                }
            }
        } else {
            if (dots_msg_recv(&bytes, sizeof(bytes), sender, NULL)) {
                abort();
            }
            assert(bytes == htonl(sender));
        }
    }

    dots_env_finalize();
}
