/**
 * \file env.h
 *
 * \brief   Stores declarations for variables for the global DoTS environment
 *          and handling environment initialization/deinitialization.
 */

#ifndef DOTS_ENV_H
#define DOTS_ENV_H

#include <stddef.h>
#include <stdint.h>
#include "dots/defs.h"

DOTS_EXTERNC_BEGIN

/**
 * \brief   Initialize DoTS runtime.
 *
 * \return  \c 0 on success.
 */
int dots_init(void);

/**
 * \brief   Destroy resources allocated for the DoTS runtime.
 */
void dots_finalize(void);

DOTS_EXTERNC_END

#endif
