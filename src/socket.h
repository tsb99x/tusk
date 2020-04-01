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

#define SERVER_BACKLOG 32
#define RECV_BUF_SIZE 8192
#define SEND_BUF_SIZE 8192

SOCKET init_listener(
        void
);

void close_listener(
        SOCKET listener
);

void accept_connections(
        SOCKET listener,
        size_t (*handler)(const char *, size_t, char *, size_t)
);

#endif
