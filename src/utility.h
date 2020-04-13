#pragma once

#include <stdio.h>

#define EPRINTF(msg, ...)                                                      \
        fprintf(stderr, "[ ERROR ] " msg "\n", ##__VA_ARGS__)

#define WPRINTF(msg, ...)                                                      \
        do {                                                                   \
                printf("[ WARN  ] " msg "\n", ##__VA_ARGS__);                  \
                fflush(stdout);                                                \
        } while(0)

#define IPRINTF(msg, ...)                                                      \
        printf("[ INFO  ] " msg "\n", ##__VA_ARGS__)

#ifndef NDEBUG
#define DPRINTF(msg, ...)                                                      \
        do {                                                                   \
                printf("[ DEBUG ] " msg "\n", ##__VA_ARGS__);                  \
                fflush(stdout);                                                \
        } while(0)
#else
#define DPRINTF(...)
#endif

#define UNUSED(x)                                                              \
        (void) x

#define SIZE_OF_ARRAY(x)                                                       \
        (sizeof(x) / sizeof(x[0]))

const char *get_env_var(
        const char *name,
        const char *def_value
);
