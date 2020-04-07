#ifndef __NODE_H__
#define __NODE_H__

enum node_type {
        LITERAL,
        VARIABLE
};

struct node {
        enum node_type type;
        const char *value;
};

#define PREFIX nodes
#define T struct node
#include "templates/pool.tpl.h"

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

#endif
