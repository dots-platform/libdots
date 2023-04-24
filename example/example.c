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

    size_t dots_world_rank = dots_env_get_world_rank();
    size_t dots_world_size = dots_env_get_world_size();

    /* Send bytes between sockets for a while for testing. */
    uint32_t bytes;
    for (size_t sender = 0; sender < dots_world_size; sender++) {
        if (dots_world_rank == sender) {
            for (int tag = 0; tag < 10; tag++) {
                bytes = htonl(sender * 10 + tag);
                for (size_t recipient = 0; recipient < dots_world_size;
                        recipient++) {
                    if (recipient == dots_world_rank) {
                        continue;
                    }

                    if (dots_msg_send(&bytes, sizeof(bytes), recipient, tag)) {
                        fprintf(stderr, "Failed sending to %zu on tag %d\n",
                                recipient, tag);
                        abort();
                    }
                    printf("Sent %zu to %zu\n", sender, recipient);
                }
            }
        } else {
            for (int tag = 9; tag >= 0; tag--) {
                if (dots_msg_recv(&bytes, sizeof(bytes), sender, tag, NULL)) {
                    fprintf(stderr, "Failed receiving from %zu on tag %d\n",
                            sender, tag);
                    abort();
                }
                if (ntohl(bytes) != sender * 10 + tag) {
                    fprintf(stderr,
                            "Received %" PRIu32 " instead of %zu from %zu on tag %d\n",
                            ntohl(bytes), sender * 10 + tag, sender, tag);
                    abort();
                }
                printf("Received %" PRIu32 " from %zu on tag %d\n",
                        ntohl(bytes), sender, tag);
            }
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
