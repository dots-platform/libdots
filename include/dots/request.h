/**
 * \file request.h
 *
 * \brief   Defines request handling methods for the DoTS application to accept
 *          and handle incoming requests from the platform.
 */

#ifndef DOTS_REQUEST_H
#define DOTS_REQUEST_H

#include <stddef.h>
#include <stdint.h>

typedef struct dots_request_arg {
    unsigned char *ptr;
    size_t length;
} dots_request_arg_t;

typedef struct dots_request {
    unsigned char id[16];
    int32_t *in_fds;
    size_t in_fds_len;
    int32_t *out_fds;
    size_t out_fds_len;
    char *func_name;
    dots_request_arg_t *args;
    size_t args_len;

    struct request_input *input;
} dots_request_t;

/**
 * \brief       Accept a new request from the DoTS server.
 *
 * \param req   A pointer to a \c dots_request_t struct that will be populated
 *              for the incoming request.
 *
 * \return      \c 0 on success.
 */
int dots_request_accept(dots_request_t *req);

/**
 * \brief       Concludes the request, returning any results back to the client.
 *
 * \param req   The request to conclude.
 *
 * \return      \c 0 on success.
 */
int dots_request_finish(dots_request_t *req);

/**
 * \brief       Free resources allocated for the request.
 *
 * \param req   The request to free.
 */
void dots_request_free(dots_request_t *req);

#endif
