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

#include "scgi.h"

#ifdef WIN32
#define EREPORT(msg) \
        EPRINTF(msg ", error: %d\n", WSAGetLastError());
#else
#define EREPORT(msg) \
        perror(msg);
#endif

SOCKET init_listener(
        int backlog_size
);

void close_listener(
        SOCKET listener
);

void accept_connections(
        SOCKET listener,
        void (*scgi_handler)(struct req_ctx *, const struct route_binding *, size_t),
        struct route_binding *routes,
        size_t routes_count,
        struct req_ctx *request
);

#endif
