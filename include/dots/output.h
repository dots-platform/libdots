#ifndef DOTS_OUTPUT_H
#define DOTS_OUTPUT_H

#include <stddef.h>
#include "dots/internal/defs.h"

/**
 * \brief           Output data to the application client.
 *
 * \param data      The buffer holding the data to be output, of at least
 *                  \c data_len bytes.
 *
 * \param data_len  The number of bytes to output.
 *
 * \return          \c 0 on success.
 */
int dots_output(const unsigned char *buf, size_t buf_len);

/**
 * \brief       Output a formatted string to the application client.
 *
 * \param fmt   The format string describing the output.
 *
 * \return      \c 0 on success.
 */
int FORMAT(printf, 1, 2) dots_outputf(const char *fmt, ...);

#endif
