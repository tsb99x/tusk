#ifndef __NODE_H__
#define __NODE_H__

#include "pool.h"

enum node_type {
        LITERAL,
        VARIABLE
};

struct node {
        enum node_type type;
        const char *value;
};

struct nodes_pool;

struct nodes_pool *init_nodes_pool(
        size_t size
);

void destroy_nodes_pool(
        struct nodes_pool *self
);

struct node *emplace_node(
        struct nodes_pool *self,
        enum node_type type,
        const char *value
);

typedef void (*nodes_iter)(struct node *, void *);

void for_each_node(
        struct nodes_pool *self,
        void *state,
        nodes_iter handler
);

void cleanup_nodes_pool(
        struct nodes_pool *self
);

#endif
