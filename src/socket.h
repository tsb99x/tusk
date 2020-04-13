#pragma once

#include <stddef.h>

#ifdef WIN32
#include <winsock2.h>
#else
typedef int SOCKET;
#endif

struct char_buf {
        char *const ptr;
        const size_t size;
        size_t count;
};

struct sock_ctx {
        struct char_buf recv;
        struct char_buf send;
};

SOCKET init_listener(
        int backlog_size
);

void close_listener(
        SOCKET listener
);

void accept_connections(
        SOCKET listener,
        void (*scgi_handler)(struct sock_ctx *),
        struct sock_ctx *ctx
);
