#ifndef DOTS_INTERNAL_DEFS_H
#define DOTS_INTERNAL_DEFS_H

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#define FORMAT(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK) __attribute__((format(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)))
#else
#error "This code must be compiled with a compiler supporting __attribute__((packed))"
#endif

#endif
