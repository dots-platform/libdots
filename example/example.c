#include "dots.h"
#include <stdlib.h>

int main(void) {
    if (dots_env_init()) {
        abort();
    }

    dots_env_finalize();
}
