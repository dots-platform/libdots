#include "dots/env.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "dots/err.h"

#define GETLINE_INITIAL_SIZE 4096
#define PARSE_INT_LINE_BUF_INITIAL_SIZE 8

size_t dots_world_rank;
size_t dots_world_size;
int *dots_comm_sockets;
int *dots_in_fds;
size_t dots_in_fds_len;
int *dots_out_fds;
size_t dots_out_fds_len;
char *dots_func_name;
int dots_control_socket;

/* Returns a malloc'd buffer containing the next line of input from f, including
 * the newline. Behaves like POSIX getline and returns SIZE_MAX upon error. */
static size_t dots_getline(char **restrict lineptr, size_t *restrict n,
        FILE *restrict stream) {
    if (!*lineptr) {
        *n = GETLINE_INITIAL_SIZE;
        *lineptr = malloc(GETLINE_INITIAL_SIZE);
    }

    size_t line_next = 0;

    /* Keep growing line until we've read the entire line in. */
    size_t line_length;
    bool newline_reached = false;
    while (!newline_reached) {
        if (!fgets(*lineptr + line_next, *n - line_next, stream)) {
            goto exit;
        }

        char *null_byte = memchr(*lineptr + line_next, '\0', *n - line_next);
        if (null_byte < *lineptr + *n - 1 || *(null_byte - 1) == '\n') {
            line_length = null_byte - *lineptr;
            newline_reached = true;
        } else {
            line_next = null_byte - *lineptr;
            *n *= 2;
            char *line_new = realloc(*lineptr, *n);
            if (!line_new) {
                goto exit;
            }
            *lineptr = line_new;
        }
    }

    return line_length;

exit:
    return SIZE_MAX;
}

/* Read a line as a string of space-separated integers, allocates a buffer *buf
 * returned by malloc(), and stores the parsed integers into the buffer and the
 * length into *len. Returns 0 upon error. *buf should be freed by free(), even
 * upon error. */
static int parse_int_line(const char *restrict line, int **restrict buf,
        size_t *restrict len) {
    *len = 0;
    size_t buflen = 0;
    const char *line_nextptr = line;
    char *line_endptr;
    int ret;
    while (line_nextptr && *line_nextptr != '\n') {
        int in_fd = strtol(line_nextptr, &line_endptr, 10);
        if (in_fd < 0 || line_endptr == line_nextptr
                || (line_endptr && *line_endptr != ' '
                    && *line_endptr != '\n')) {
            ret = DOTS_ERR_INTERFACE;
            goto exit;
        }

        if (buflen == *len) {
            buflen = buflen ? buflen * 2 : PARSE_INT_LINE_BUF_INITIAL_SIZE;
            int *new_buf = realloc(*buf, buflen * sizeof(**buf));
            if (!new_buf) {
                ret = DOTS_ERR_LIBC;
                goto exit;
            }
            *buf = new_buf;
        }

        (*len)++;
        (*buf)[*len - 1] = in_fd;
        line_nextptr = line_endptr;
    }
    int *new_buf = realloc(*buf, *len * sizeof(*buf));
    if (!new_buf) {
        ret = DOTS_ERR_LIBC;
        goto exit;
    }
    *buf = new_buf;

    ret = 0;

exit:
    return ret;
}

int dots_env_init(void) {
    char *line = NULL;
    size_t line_buflen;
    size_t line_len;
    char *line_endptr;
    long long parsed_int;
    int ret;

    /* Parse world rank. */
    line_len = dots_getline(&line, &line_buflen, stdin);
    if (line_len == SIZE_MAX) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_line;
    }
    parsed_int = strtoll(line, &line_endptr, 10);
    if (parsed_int < 0 || line_endptr == line
            || (line_endptr && *line_endptr != '\n')) {
        ret = DOTS_ERR_INTERFACE;
        goto exit_free_line;
    }
    dots_world_rank = parsed_int;

    /* Parse input FDs. */
    line_len = dots_getline(&line, &line_buflen, stdin);
    if (line_len == SIZE_MAX) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_line;
    }
    ret = parse_int_line(line, &dots_in_fds, &dots_in_fds_len);
    if (ret) {
        goto exit_free_in_fds;
    }

    /* Parse output FDs. */
    line_len = dots_getline(&line, &line_buflen, stdin);
    if (line_len == SIZE_MAX) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_in_fds;
    }
    ret = parse_int_line(line, &dots_out_fds, &dots_out_fds_len);
    if (ret) {
        goto exit_free_out_fds;
    }

    /* Parse communication sockets. */
    line_len = dots_getline(&line, &line_buflen, stdin);
    if (line_len == SIZE_MAX) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_out_fds;
    }
    ret = parse_int_line(line, &dots_comm_sockets, &dots_world_size);

    /* Get function name. We'll use the line buffer for this. */
    line_len = dots_getline(&line, &line_buflen, stdin);
    if (line_len == SIZE_MAX) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_comm_sockets;
    }
    if (line[line_len - 1] == '\n') {
        line[line_len - 1] = '\0';
        line_len--;
    }
    char *new_line = realloc(line, line_len + 1);
    if (!new_line) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_comm_sockets;
    }
    line = new_line;
    dots_func_name = line;

    /* Open control socket. */
    // TODO Some more sane path handling here would be nice, or the DoTS server
    // could just use a socketpair rather than relying on a socket in /tmp.
    dots_control_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (dots_control_socket < 0) {
        ret = DOTS_ERR_LIBC;
        goto exit_free_comm_sockets;
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
exit_free_comm_sockets:
    free(dots_comm_sockets);
exit_free_out_fds:
    free(dots_out_fds);
exit_free_in_fds:
    free(dots_in_fds);
exit_free_line:
    free(line);
    return ret;
}

void dots_env_finalize(void) {
    free(dots_in_fds);
    free(dots_out_fds);
    free(dots_comm_sockets);
    free(dots_func_name);
}
