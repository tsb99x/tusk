#include <stdio.h>
#include <stdlib.h>

#include "node.h"

#define PREFIX nodes
#define T struct node
#include "templates/pool.tpl.c"

struct node *emplace_node(
        struct nodes_pool *self,
        enum node_type type,
        const char *value
) {
        struct node *ptr = self->last;
        ptr -> type = type;
        ptr -> value = value;
        if (++self->last >= self->limit) {
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
