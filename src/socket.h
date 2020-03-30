#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifdef WIN32
#include <winsock2.h>
#else
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

#ifdef WIN32
#define EREPORT(msg) \
        EPRINTF(msg ", error: %d\n", WSAGetLastError());
#else
#define EREPORT(msg) \
        perror(msg);
#endif

SOCKET init_listener(
        void
);

void close_listener(
        SOCKET socket
);

#endif