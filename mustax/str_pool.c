#include <string.h>

#include "str_pool.h"

#define PREFIX str
#define T char
#include "templates/pool.tpl.c"

char *copy_into_pool(
        struct str_pool *self,
        const char *str,
        size_t len
) {
        char *ptr = self->last;
        if (ptr + len >= self->limit) {
                fprintf(stderr, "Buffer is exhausted\n");
                exit(EXIT_FAILURE);
        }
        memcpy(ptr, str, len);
        self->last += len;
        *self->last = '\0';
        self->last++;
        return ptr;
}
