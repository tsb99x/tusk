#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include "utility.h"

#include "socket.h"

void init_winsock(
        void
) {
        #ifdef WIN32
        WSADATA wsa;
        WORD version = MAKEWORD(2, 2);
        if (WSAStartup(version, &wsa) != 0) {
                EREPORT("Failed to init WinSock");
                exit(EXIT_FAILURE);
        }
        DPRINTF("* WinSock initialized\n");
        #endif
}

void cleanup_winsock(
        void
) {
        #ifdef WIN32
        WSACleanup();
        DPRINTF("* WinSock cleanup\n");
        #endif
}

SOCKET create_socket(
        void
) {
        SOCKET new_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (new_socket == INVALID_SOCKET) {
                EREPORT("Failed to create socket");
                cleanup_winsock();
                exit(EXIT_FAILURE);
        }
        DPRINTF("* Socket created\n");
        return new_socket;
}

void close_socket(
        SOCKET socket
) {
        #ifdef WIN32
        closesocket(socket);
        #else
        close(socket);
        #endif
        DPRINTF("* Uninitialized socket\n");
}

struct sockaddr_in build_server_data(
        void
) {
        int http_port = atoi(get_env_var("HTTP_PORT", "8888"));
        struct sockaddr_in server_data = {
                .sin_family = AF_INET,
                .sin_addr.s_addr = INADDR_ANY,
                .sin_port = htons(http_port)
        };
        return server_data;
}

void bind_socket(
        SOCKET socket,
        struct sockaddr *server
) {
        if (bind(socket, server, sizeof(struct sockaddr)) == SOCKET_ERROR) {
                EREPORT("Failed to bind socket");
                close_listener(socket);
                exit(EXIT_FAILURE);
        }
        DPRINTF("* Socket bound\n");
}

SOCKET init_listener(
        void
) {
        init_winsock();
        SOCKET socket = create_socket();
        struct sockaddr_in server = build_server_data();
        bind_socket(socket, (struct sockaddr *) &server);
        return socket;
}

void close_listener(
        SOCKET socket
) {
        close_socket(socket);
        cleanup_winsock();
}
