#ifndef DOTS_OUTPUT_H
#define DOTS_OUTPUT_H

#include <stddef.h>
#include "dots/request.h"
#include "dots/internal/defs.h"

/**
 * \brief           Output data to the application client.
 *
 * \param req       The request associated with this output.
 *
 * \param data      The buffer holding the data to be output, of at least
 *                  \c data_len bytes.
 *
 * \param data_len  The number of bytes to output.
 *
 * \return          \c 0 on success.
 */
int dots_output(dots_request_t *req,
        const unsigned char *buf, size_t buf_len);

/**
 * \brief       Output a formatted string to the applicatiosendn client.
 *
 * \param req   The request associated with this output.
 *
 * \param fmt   The format string describing the output.
 *
 * \return      \c 0 on success.
 */
int FORMAT(printf, 2, 3) dots_outputf(dots_request_t *req, const char *fmt,
        ...);

#endif
