#include <stdio.h>
#include <stdlib.h>

#include "node.h"

struct nodes_pool {
        POOL_STRUCT(struct node);
};

// #define NODES_MEM_SIZE 256
// struct node __nodes_mem[NODES_MEM_SIZE];
// struct nodes_pool nodes = INIT_POOL(__nodes_mem, NODES_MEM_SIZE);

struct nodes_pool *init_nodes_pool(
        size_t pool_size
) {
        struct node *mem = malloc(pool_size * sizeof(struct node));
        if (mem == NULL) {
                // FIXME : PANIC HERE
                exit(EXIT_FAILURE);
        }
        struct nodes_pool *pool = malloc(sizeof(struct nodes_pool));
        if (pool == NULL) {
                // FIXME : PANIC HERE
                exit(EXIT_FAILURE);
        }
        pool->ptr = mem;
        pool->last = mem;
        pool->limit = mem + pool_size;
        return pool;
}

void destroy_nodes_pool(
        struct nodes_pool *self
) {
        free(self->ptr);
        free(self);
}

struct node *emplace_node(
        struct nodes_pool *self,
        enum node_type type,
        const char *value
) {
        struct node *ptr = self -> last;
        ptr -> type = type;
        ptr -> value = value;
        if (++self->last >= self -> limit) {
                fprintf(stderr, "Buffer is exhausted\n");
                exit(EXIT_FAILURE);
        }
        return ptr;
}

void for_each_node(
        struct nodes_pool *self,
        void *state,
        nodes_iter handler
) {
        for (struct node *it = self->ptr; it < self->last; it++)
                handler(it, state);
}

void cleanup_nodes_pool(
        struct nodes_pool *self
) {
        self->last = self->ptr;
}
