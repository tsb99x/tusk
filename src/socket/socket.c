#include <socket.h>

#include <utility.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include <stdlib.h>

#ifdef WIN32
typedef int socklen_t;
#else
typedef unsigned int socklen_t;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#ifdef WIN32
#define EREPORT(msg) \
        EPRINTF(msg ", error: %d", WSAGetLastError());
#else
#define EREPORT(msg) \
        perror(msg);
#endif

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
        DPRINTF("WinSock initialized");
        #endif
}

void cleanup_winsock(
        void
) {
        #ifdef WIN32
        WSACleanup();
        DPRINTF("WinSock cleanup");
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
        DPRINTF("Socket created");
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
        DPRINTF("Socket closed");
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
        DPRINTF("Socket bound");
}

void enable_listener(
        SOCKET socket,
        int backlog_size
) {
        if (listen(socket, backlog_size) != NO_ERROR) {
                EREPORT("Failed to listen on socket");
                close_listener(socket);
                exit(EXIT_FAILURE);
        }
        DPRINTF("Socket listening");
}

SOCKET init_listener(
        int backlog_size
) {
        init_winsock();
        SOCKET socket = create_socket();
        struct sockaddr_in server_addr = build_server_data();
        bind_socket(socket, (struct sockaddr *) &server_addr);
        enable_listener(socket, backlog_size);
        return socket;
}

void close_listener(
        SOCKET listener
) {
        close_socket(listener);
        cleanup_winsock();
}

int receive_data(
        SOCKET client_socket,
        struct char_buf *recv_buf
) {
        int bytes = recv(client_socket, recv_buf->ptr, (int) recv_buf->size, 0);
        if (bytes >= 0)
                recv_buf->count = bytes;
        DPRINTF("Received request with size of %d byte(s)", bytes);
        return bytes;
}

void send_data(
        SOCKET client_socket,
        struct char_buf *send_buf
) {
        if (send_buf->count == 0)
                return;
        send(client_socket, send_buf->ptr, (int) send_buf->count, 0);
        DPRINTF("Sent response with size of %zu byte(s)", send_buf->count);
}

void accept_connections(
        SOCKET listener,
        void (*scgi_handler)(struct sock_ctx *),
        struct sock_ctx *ctx
) {
        struct sockaddr client_addr;
        size_t sockaddr_size = sizeof(struct sockaddr);
        SOCKET client_socket;
        DPRINTF("Waiting for connections");
        while ((client_socket = accept(listener, &client_addr, (socklen_t *) &sockaddr_size)) != INVALID_SOCKET) {
                DPRINTF("Connection accepted");
                int recv_bytes = receive_data(client_socket, &ctx->recv);
                if (recv_bytes > 0) {
                        (*scgi_handler)(ctx);
                        send_data(client_socket, &ctx->send);
                } else if (recv_bytes == 0) {
                        DPRINTF("Connection closed by client");
                } else {
                        EREPORT("Failed to receive data");
                }
                close_socket(client_socket);
        }
        EREPORT("Failed to accept connection");
        close_listener(listener);
        exit(EXIT_FAILURE);
}
