#include "dots/output.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "dots/err.h"
#include "dots/internal/control_msg.h"

#define INITIAL_OUTPUTF_BUF_SIZE 4096

int dots_output(const unsigned char *data, size_t data_len) {
    int ret;

    struct control_msg msg;
    ret = dots_send_control_msg(&msg, CONTROL_MSG_TYPE_OUTPUT, data, data_len);
    if (ret) {
        goto exit;
    }

exit:
    return ret;
}

int dots_outputf(const char *fmt, ...) {
    int ret;

    va_list args;
    va_start(args, fmt);

    char *buf = NULL;
    size_t buf_size = INITIAL_OUTPUTF_BUF_SIZE;
    while (1) {
        /* Grow (or allocate) buffer. */
        char *new_buf = realloc(buf, buf_size);
        if (!new_buf) {
            ret = DOTS_ERR_LIBC;
            goto exit_free_buf;
        }
        buf = new_buf;

        /* Attempt to write format string to buffer. */
        ret = vsnprintf(buf, buf_size, fmt, args);
        if (ret < 0) {
            ret = DOTS_ERR_LIBC;
            goto exit_free_buf;
        }
        if ((size_t) ret == buf_size) {
            continue;
        }
        size_t data_len = ret;

        /* Write output and break. */
        ret = dots_output((unsigned char *) buf, data_len);
        if (ret) {
            goto exit_free_buf;
        }
    }

    ret = 0;

exit_free_buf:
    free(buf);
    return ret;
}
