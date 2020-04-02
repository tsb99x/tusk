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
char recv_buf[RECV_BUF_SIZE];

#define SEND_BUF_SIZE 8192
char send_buf[SEND_BUF_SIZE];

SOCKET init_listener(
        void
);

void close_listener(
        SOCKET listener
);

void accept_connections(
        SOCKET listener,
        size_t (*handler)(const char *, const char *, char *, size_t),
        char *recv_buf,
        size_t recv_buf_size,
        char *send_buf,
        size_t send_buf_size
);

#endif
