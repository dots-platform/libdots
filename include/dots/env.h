/**
 * \file env.h
 *
 * \brief   Stores declarations for variables for the global DoTS environment
 *          and handling environment initialization/deinitialization.
 */

#ifndef DOTS_ENV_H
#define DOTS_ENV_H

#include <stddef.h>

/**
 * \brief   Rank of the DoTS application.
 */
extern size_t dots_world_rank;

/**
 * \brief   Number of nodes in the DoTS application.
 */
extern size_t dots_world_size;

/**
 * \brief   Sockets used for inter-node communication by reading and writing.
 */
extern int *dots_comm_sockets;

/**
 * \brief   Read-only file descriptors for reading input files.
 */
extern int *dots_in_fds;

/**
 * \brief   Length of \c dots_in_fds.
 */
extern size_t dots_in_fds_len;

/**
 * \brief   Write-only file descriptors for writing output files.
 */
extern int *dots_out_fds;

/**
 * \brief   Length of \c dots_out_fds.
 */
extern size_t dots_out_fds_len;

/**
 * \brief   Name of the called function.
 */
extern char *dots_func_name;

/**
 * \brief   The control socket to the DoTS server.
 */
extern int dots_control_socket;

/**
 * \brief   Initialize DoTS environment from the program's startup environment.
 *
 * \note    This function must be called before any other function that changes
 *          the state of the program. Notably, the program must not read from
 *          stdin before calling this function.
 *
 * \return  \c 0 on success.
 */
int dots_env_init(void);

/**
 * \brief   Destroy resources allocated for the DoTS environment.
 */
void dots_env_finalize(void);

#endif
