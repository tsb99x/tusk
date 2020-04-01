#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdio.h>

#define EPRINTF(...) \
        fprintf(stderr, "[ ERROR ] " __VA_ARGS__)

#define WPRINTF(...) \
        printf("[ WARN  ] " __VA_ARGS__)

#ifndef NDEBUG
#define DPRINTF(...)                              \
        do {                                      \
                printf("[ DEBUG ] " __VA_ARGS__); \
                fflush(stdout);                   \
        } while(0)
#else
#define DPRINTF(...)
#endif

#define UNUSED(x) \
        (void) x

const char *get_env_var(
        const char *name,
        const char *def_value
);

#endif
