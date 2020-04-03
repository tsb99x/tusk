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
        SOCKET socket,
        int backlog_size
) {
        if (listen(socket, backlog_size) != 0) {
                EREPORT("Failed to listen on socket");
                close_listener(socket);
                exit(EXIT_FAILURE);
        }
        DPRINTF("Socket listening\n");
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

void process_data(
        SOCKET client_socket,
        void (*scgi_handler)(struct req_ctx *, const struct route_binding *, size_t),
        struct route_binding *routes,
        size_t routes_count,
        struct req_ctx *request
) {
        size_t recv_bytes = recv(client_socket, request->recv_buf, (int) request->recv_buf_size, 0);
        if (recv_bytes > 0) {
                DPRINTF("Received request with size of %zu byte(s)\n", recv_bytes);
                request->recv_count = recv_bytes;
                (*scgi_handler)(request, routes, routes_count);
                if (request->send_count > 0) {
                        send(client_socket, request->send_buf, (int) request->send_count, 0);
                        DPRINTF("Sent response with size of %zu byte(s)\n", request->send_count);
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
        void (*scgi_handler)(struct req_ctx *, const struct route_binding *, size_t),
        struct route_binding *routes,
        size_t routes_count,
        struct req_ctx *request
) {
        struct sockaddr client_addr;
        size_t sockaddr_size = sizeof(struct sockaddr);
        SOCKET client_socket;
        DPRINTF("Waiting for connections\n");
        while ((client_socket = accept(listener, &client_addr, (socklen_t *) &sockaddr_size)) != INVALID_SOCKET) {
                DPRINTF("Connection accepted\n");
                process_data(client_socket, scgi_handler, routes, routes_count, request);
        }
        EREPORT("Failed to accept connection");
        close_listener(listener);
        exit(EXIT_FAILURE);
}
