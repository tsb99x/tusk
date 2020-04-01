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
        DPRINTF("WinSock initialized\n");
        #endif
}

void cleanup_winsock(
        void
) {
        #ifdef WIN32
        WSACleanup();
        DPRINTF("WinSock cleanup\n");
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
        DPRINTF("Socket created\n");
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
        DPRINTF("Socket closed\n");
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
        const struct sockaddr *server
) {
        size_t sockaddr_size = sizeof(struct sockaddr);
        if (bind(socket, server, (int) sockaddr_size) == SOCKET_ERROR) {
                EREPORT("Failed to bind socket");
                close_listener(socket);
                exit(EXIT_FAILURE);
        }
        DPRINTF("Socket bound\n");
}

void enable_listener(
        SOCKET socket
) {
        if (listen(socket, SERVER_BACKLOG) != 0) {
                EREPORT("Failed to listen on socket");
                close_listener(socket);
                exit(EXIT_FAILURE);
        }
        DPRINTF("Socket listening\n");
}

SOCKET init_listener(
        void
) {
        init_winsock();
        SOCKET socket = create_socket();
        struct sockaddr_in server_addr = build_server_data();
        bind_socket(socket, (struct sockaddr *) &server_addr);
        enable_listener(socket);
        return socket;
}

void close_listener(
        SOCKET listener
) {
        close_socket(listener);
        cleanup_winsock();
}

void receive_data(
        SOCKET client_socket,
        size_t (*handler)(const char *, const char *, char *, size_t)
) {
        char recv_buf[RECV_BUF_SIZE];
        int recv_bytes = recv(client_socket, recv_buf, RECV_BUF_SIZE, 0);
        char send_buf[SEND_BUF_SIZE];
        if (recv_bytes > 0) {
                DPRINTF("Received request with size of %d byte(s)\n", recv_bytes);
                size_t send_bytes = (*handler)(recv_buf, recv_buf + recv_bytes, send_buf, SEND_BUF_SIZE);
                if (send_bytes > 0) {
                        send(client_socket, send_buf, (int) send_bytes, 0);
                        DPRINTF("Sent response with size of %zu byte(s)\n", send_bytes);
                }
        } else if (recv_bytes == 0) {
                DPRINTF("Connection closed by client\n");
        } else {
                EREPORT("Failed to receive data");
        }
        close_socket(client_socket);
}

void accept_connections(
        SOCKET listener,
        size_t (*handler)(const char *, const char *, char *, size_t)
) {
        struct sockaddr client_addr;
        size_t sockaddr_size = sizeof(struct sockaddr);
        SOCKET client_socket;
        DPRINTF("Waiting for connections\n");
        while ((client_socket = accept(listener, &client_addr, (socklen_t *) &sockaddr_size)) != INVALID_SOCKET) {
                DPRINTF("Connection accepted\n");
                receive_data(client_socket, handler);
        }
        if (client_socket == INVALID_SOCKET) {
                EREPORT("Failed to accept connection");
                close_listener(listener);
                exit(EXIT_FAILURE);
        }
}
