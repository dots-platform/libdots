/**
 * \file env.h
 *
 * \brief   Stores declarations for variables for the global DoTS environment
 *          and handling environment initialization/deinitialization.
 */

#ifndef DOTS_ENV_H
#define DOTS_ENV_H

#include <stddef.h>
#include "dots/defs.h"

DOTS_EXTERNC_BEGIN

typedef struct dots_env_arg {
    unsigned char *ptr;
    size_t length;
} dots_env_arg_t;

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

/**
 * \brief   Return the rank of the DoTS application.
 *
 * \return  The rank of the DoTS application.
 */
size_t dots_env_get_world_rank(void);

/**
 * \brief   Return the number of nodes in the DoTS application.
 *
 * \return  The number of nodes in the DoTS application.
 */
size_t dots_env_get_world_size(void);

/**
 * \brief       Fetch the list of read-only file descriptors for reading
 *              input files.
 *
 * \param fds   A pointer to a buffer of FDs which the input FDs will be written
 *              to. This buffer must be of length at least
 *              \c dots_env_get_num_in_files().
 */
void dots_env_get_in_fds(int *fds);

/**
 * \brief   Return the number of input files.
 *
 * \return  The number of input files.
 */
size_t dots_env_get_num_in_files(void);

/**
 * \brief       Fetch the list of write-only file descriptors for writing
 *              output files.
 *
 * \param fds   A pointer to a buffer of FDs which the output FDs wil be written
 *              to. This buffer must be of length at least
 *              \c dots_env_get_num_out_files().
 */
void dots_env_get_out_fds(int *fds);

/**
 * \brief   Return the number of output files.
 *
 * \return  The number of output files.
 */
size_t dots_env_get_num_out_files(void);

/**
 * \brief   Returns the name of the called function.
 *
 * \return  The name of the called function.
 */
const char *dots_env_get_func_name(void);

/**
 * \brief       Return the arguments passed to the DoTS application.
 *
 * \param args  A pointer to a buffer of \c dots_env_arg_t structs which the
 *              arguments will be written to This buffer must be of length at
 *              least \c dots_env_get_num_args().
 */
void dots_env_get_args(dots_env_arg_t *args);

/**
 * \brief   Return the number of arguments.
 *
 * \return  The number of arguments.
 */
size_t dots_env_get_num_args(void);

DOTS_EXTERNC_END

#endif
