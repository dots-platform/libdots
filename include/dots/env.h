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

/**
 * \brief   Initialize DoTS environment from the program's startup environment.
 *
 * \note    This function must be called before any other function that changes
 *          the state of the program. Notably, the program must not read from
 *          stdin before calling this function.
 *
 * \return  \c 0 on success.
 */
int dots_init(void);

/**
 * \brief   Destroy resources allocated for the DoTS runtime.
 */
void dots_finalize(void);

/**
 * \brief   Return the rank of the DoTS application.
 *
 * \return  The rank of the DoTS application.
 */
size_t dots_get_world_rank(void);

/**
 * \brief   Return the number of nodes in the DoTS application.
 *
 * \return  The number of nodes in the DoTS application.
 */
size_t dots_get_world_size(void);

DOTS_EXTERNC_END

#endif
