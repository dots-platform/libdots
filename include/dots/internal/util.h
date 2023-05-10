#ifndef DOTS_INTERNAL_UTIL_H
#define DOTS_INTERNAL_UTIL_H

#include <stdint.h>

static inline uint64_t dots_htonll(uint64_t hostlonglong) {
    union {
        uint64_t val;
        unsigned char bytes[8];
    } u = {
        .bytes = {
            hostlonglong >> 56,
            (hostlonglong >> 48) & 0xff,
            (hostlonglong >> 40) & 0xff,
            (hostlonglong >> 32) & 0xff,
            (hostlonglong >> 24) & 0xff,
            (hostlonglong >> 16) & 0xff,
            (hostlonglong >> 8) & 0xff,
            hostlonglong & 0xff,
        },
    };
    return u.val;
}

static inline uint64_t dots_ntohll(uint64_t netlonglong) {
    union {
        uint64_t val;
        unsigned char bytes[8];
    } u = {
        .val = netlonglong,
    };
    return ((uint64_t) u.bytes[0] << 56)
        | ((uint64_t) u.bytes[1] << 48)
        | ((uint64_t) u.bytes[2] << 40)
        | ((uint64_t) u.bytes[3] << 32)
        | ((uint64_t) u.bytes[4] << 24)
        | ((uint64_t) u.bytes[5] << 16)
        | ((uint64_t) u.bytes[6] << 8)
        | (uint64_t) u.bytes[7];
}

#endif
