#include "dots/env.h"

int dots_control_socket;

int dots_init(void) {
    int ret;

    dots_control_socket = 3;

    ret = 0;

    return ret;
}

void dots_finalize(void) {}
