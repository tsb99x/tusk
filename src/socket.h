#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifdef WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
typedef unsigned int socklen_t;
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#ifdef WIN32
#define EREPORT(msg) \
        EPRINTF(msg ", error: %d\n", WSAGetLastError());
#else
#define EREPORT(msg) \
        perror(msg);
#endif

struct sock_ctx {
        char *const recv_buf;
        const size_t recv_buf_size;
        size_t recv_count;

        char *const send_buf;
        const size_t send_buf_size;
        size_t send_count;
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

#endif
