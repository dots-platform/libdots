#include "dots.h"
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    if (dots_env_init()) {
        fprintf(stderr, "Failed environment initialization\n");
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
                    fprintf(stderr, "Failed sending to %zu\n", recipient);
                    abort();
                }
                printf("Sent %zu to %zu\n", sender, recipient);
                if (dots_world_rank == recipient) {
                    if (dots_msg_recv(&bytes, sizeof(bytes), sender, NULL)) {
                        fprintf(stderr, "Failed receiving from self\n");
                        abort();
                    }
                    if (ntohl(bytes) != sender) {
                        fprintf(stderr,
                                "Received %" PRIu32 " instead of %zu from self\n",
                                ntohl(bytes), sender);
                    }
                    printf("Received %" PRIu32 " from self\n", ntohl(bytes));
                }
            }
        } else {
            if (dots_msg_recv(&bytes, sizeof(bytes), sender, NULL)) {
                fprintf(stderr, "Failed receiving from %zu\n", sender);
                abort();
            }
            if (ntohl(bytes) != sender) {
                fprintf(stderr,
                        "Received %" PRIu32 " instead of %zu from %zu\n",
                        ntohl(bytes), sender, sender);
                abort();
            }
            printf("Received %" PRIu32 " from %zu\n", ntohl(bytes), sender);
        }
    }

    /* Open a second socket. */
    if (dots_world_size >= 2) {
        if (dots_world_rank == 0) {
            int socket = dots_open_socket(1);
            if (socket < 0) {
                fprintf(stderr, "Failed to open socket to 0\n");
                abort();
            }
            printf("Opened socket %d to %d\n", socket, 1);
        } else if (dots_world_rank == 1) {
            int socket = dots_open_socket(0);
            if (socket < 0) {
                fprintf(stderr, "Failed to open socket to 1\n");
                abort();
            }
            printf("Opened socket %d to %d\n", socket, 0);
        }
    }

    dots_env_finalize();
}
