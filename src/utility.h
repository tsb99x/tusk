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

char *get_env_var(
        char *name,
        char *def_value
);

#endif
