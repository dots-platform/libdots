#include <arpa/inet.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <dots.h>

static int handle_request(void) {
    int ret;

    dots_request_t req;
    if (dots_request_accept(&req)) {
        fprintf(stderr, "Failed to accept request\n");
        ret = -1;
        goto exit;
    }

    /* Send bytes between sockets for a while for testing. */
    uint32_t bytes;
    for (size_t sender = 0; sender < req.world_size; sender++) {
        if (req.world_rank == sender) {
            for (int tag = 0; tag < 10; tag++) {
                bytes = htonl(sender * 10 + tag);
                for (size_t recipient = 0; recipient < req.world_size;
                        recipient++) {
                    if (recipient == req.world_rank) {
                        continue;
                    }

                    if (dots_msg_send(&req, &bytes, sizeof(bytes), recipient, tag)) {
                        fprintf(stderr, "Failed sending to %zu on tag %d\n",
                                recipient, tag);
                        ret = -1;
                        goto exit_free_request;
                    }
                    printf("Sent %zu to %zu\n", sender, recipient);
                }
            }
        } else {
            for (int tag = 9; tag >= 0; tag--) {
                if (dots_msg_recv(&req, &bytes, sizeof(bytes), sender, tag, NULL)) {
                    fprintf(stderr, "Failed receiving from %zu on tag %d\n",
                            sender, tag);
                    ret = -1;
                    goto exit_free_request;
                }
                if (ntohl(bytes) != sender * 10 + tag) {
                    fprintf(stderr,
                            "Received %" PRIu32 " instead of %zu from %zu on tag %d\n",
                            ntohl(bytes), sender * 10 + tag, sender, tag);
                    ret = -1;
                    goto exit_free_request;
                }
                printf("Received %" PRIu32 " from %zu on tag %d\n",
                        ntohl(bytes), sender, tag);
            }
        }
    }

    /* Read arguments and dump them out. */
    for (size_t i = 0; i < req.args_len; i++) {
        printf("arg[%zu] as string: %s\n", i, (char *) req.args[i].ptr);
        printf("arg[%zu] as bytes: ", i);
        for (size_t j = 0; j < req.args[i].length; j++) {
            printf("%02x", req.args[i].ptr[j]);
        }
        printf("\n");
    }

    /* Test some output. */
    if (dots_outputf(&req, "Hello world!\n")) {
        fprintf(stderr, "Failed to outputf static string\n");
        ret = -1;
        goto exit_free_request;
    }
    if (dots_outputf(&req, "%s %d %d %d\n", "foobar", 1, 2, 3)) {
        fprintf(stderr, "Failed to outputf formatted string\n");
        ret = -1;
        goto exit_free_request;
    }
    unsigned char buf[] = { 1, 2, 3, 4, 5 };
    if (dots_output(&req, buf, sizeof(buf))) {
        fprintf(stderr, "Failed to output raw buffer\n");
        ret = -1;
        goto exit_free_request;
    }

    if (dots_request_finish(&req)) {
        fprintf(stderr, "Failed to finish request\n");
        ret = -1;
        goto exit_free_request;
    }

    ret = 0;

exit_free_request:
    dots_request_free(&req);
exit:
    return ret;
}

int main(void) {
    if (dots_init()) {
        fprintf(stderr, "Failed runtime initialization\n");
        abort();
    }

    while (1) {
        if (handle_request()) {
            abort();
        }
    }
}
