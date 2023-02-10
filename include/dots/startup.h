#ifndef DOTS_STARTUP_H
#define DOTS_STARTUP_H

/**
 * \file startup.h
 *
 * \brief   Program startup handling to initialize local DoTS context.
 */

/**
 * \brief   Initialize DoTS context from the program's startup environment.
 *
 * \note    This function must be called before any other function that changes
 *          the state of the program. Notably, the program must not read from
 *          stdin before calling this function.
 *
 * \return  \c 0 on success.
 */
int dots_init(void);

#endif
