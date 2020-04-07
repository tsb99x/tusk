#include <stdlib.h>
#include <stdio.h>

#ifndef PREFIX
#error "No PREFIX provided for pool template"
#endif

#ifndef T
#error "No T (base type) provided for pool template"
#endif

#define CONCAT(a, b) a ## b
#define EVAL(x, y) CONCAT(x, y)
#define S EVAL(PREFIX, _pool)

struct S {
        T *ptr;
        T *last;
        T *limit;
};

struct S *EVAL(init_,S)(
        size_t size
) {
        T *mem = malloc(size * sizeof(T));
        if (mem == NULL) {
                fprintf(stderr, "Failed to allocate memory for pool data");
                exit(EXIT_FAILURE);
        }
        struct S *pool = malloc(sizeof(struct S));
        if (pool == NULL) {
                fprintf(stderr, "Failed to allocate memory for pool struct");
                exit(EXIT_FAILURE);
        }
        pool->ptr = mem;
        pool->last = mem;
        pool->limit = mem + size;
        return pool;
}

void EVAL(destroy_,S)(
        struct S *self
) {
        free(self->ptr);
        free(self);
}

void EVAL(cleanup_,S)(
        struct S *self
) {
        self->last = self->ptr;
}

#undef T
#undef PREFIX
