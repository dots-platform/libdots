#ifndef DOTS_INTERNAL_ENV_H
#define DOTS_INTERNAL_ENV_H

#include <stddef.h>
#include <stdint.h>

extern size_t dots_world_rank;
extern size_t dots_world_size;
extern int32_t *dots_in_fds;
extern size_t dots_in_fds_len;
extern int32_t *dots_out_fds;
extern size_t dots_out_fds_len;
extern char *dots_func_name;
extern int dots_control_socket;

#endif
