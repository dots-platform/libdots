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

    /* Read arguments and dump them out. */
    size_t num_args = dots_env_get_num_args();
    dots_env_arg_t *args = malloc(num_args * sizeof(*args));
    if (!args) {
        perror("malloc args");
        abort();
    }
    dots_env_get_args(args);
    for (size_t i = 0; i < num_args; i++) {
        printf("arg[%zu] as string: %s\n", i, (char *) args[i].ptr);
        printf("arg[%zu] as bytes: ", i);
        for (size_t j = 0; j < args[i].length; j++) {
            printf("%02x", args[i].ptr[j]);
        }
        printf("\n");
    }

    /* Test some output. */
    if (dots_outputf("Hello world!\n")) {
        fprintf(stderr, "Failed to outputf static string\n");
        abort();
    }
    if (dots_outputf("%s %d %d %d\n", "foobar", 1, 2, 3)) {
        fprintf(stderr, "Failed to outputf formatted string\n");
        abort();
    }
    unsigned char buf[] = { 1, 2, 3, 4, 5 };
    if (dots_output(buf, sizeof(buf))) {
        fprintf(stderr, "Failed to output raw buffer\n");
        abort();
    }

    dots_env_finalize();
}
