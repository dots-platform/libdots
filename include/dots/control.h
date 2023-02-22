/**
 * \file control.h
 *
 * \brief   Control routines for interacting with the DoTS server.
 */

#ifndef DOTS_CONTROL_H
#define DOTS_CONTROL_H

#include <stddef.h>
#include "dots/defs.h"

DOTS_EXTERNC_BEGIN

/**
 * \brief               Requests an additional socket connected to another DoTS
 *                      application node. The other node must also call
 *                      \c dots_request_sock with this node as the argument to
 *                      complete the connection.
 *
 * \param other_rank    The rank of the other end that the socket should be
 *                      connected to.
 *
 * \return              The requested socket on success, or a negative number
 *                      error code on failure.
 */
int dots_open_socket(size_t other_rank);

DOTS_EXTERNC_END

#endif
