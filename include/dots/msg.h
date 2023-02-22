/**
 * \file msg.h
 *
 * \brief   Message sending and receiving facilities for DoTS applications to
 *          communicate between nodes.
 */

#ifndef DOTS_MSG_H
#define DOTS_MSG_H

#include <stddef.h>

/**
 * \brief           Send a message to another DoTS application node.
 *
 * \param buf       The message to send.
 *
 * \param len       The length of the message, in bytes.
 *
 * \param recipient The ID of the the recipient node.
 *
 * \return          \c 0 on success.
 */
int dots_msg_send(const void *buf, size_t len, int recipient);

/**
 * \brief           Receive a message from another DoTS application node.
 *
 * \param buf       The buffer which the message will be written to.
 *
 * \param len       The maximum length of the message, in bytes.
 *
 * \param recipient The ID of the of sending node.
 *
 * \param recv_len  A pointer which will hold the total number of bytes received
 *                  and written to \c buf. May be \c NULL.
 *
 * \return          \c 0 on success.
 */
int dots_msg_recv(void *buf, size_t len, int sender, size_t *recv_len);

#endif
