#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "utility.h"
#include "socket.h"
#include "scgi.h"

#define BACKLOG_SIZE 32

#define RECV_BUF_SIZE 8192
char recv_buf[RECV_BUF_SIZE];

#define SEND_BUF_SIZE 8192
char send_buf[SEND_BUF_SIZE];

#define HEADERS_BUF_SIZE 64
struct sz_pair headers_buf[HEADERS_BUF_SIZE];

#define RAW_SZ_FORM_DATA_BUF_SIZE 4096
char raw_sz_form_data[RAW_SZ_FORM_DATA_BUF_SIZE];

#define FORM_DATA_BUF_SIZE 64
struct sz_pair form_data[FORM_DATA_BUF_SIZE];

SOCKET listener;

void shutdown_handler(
        int signum
) {
        UNUSED(signum);
        puts("Shutdown in progress");
        close_listener(listener);
}

void hello_handler(
        struct req_ctx *request
) {
        IPRINTF("Handling /hello\n");

        // x_www_form_urlencoded_decode(
        //         it, it_end, 
        //         raw_sz_form_data, raw_sz_form_data + RAW_SZ_FORM_DATA_BUF_SIZE,
        //         form_data, FORM_DATA_BUF_SIZE
        // );

        respond_sz(request,
                "Status: 200 OK\r\n"                \
                "Content-Type: text/plain\r\n"      \
                "\r\n"                              \
                "Hello from Tusk");
}

struct route_binding routes[] = {
        {
                .path = "/hello",
                .method = "GET",
                .accepts = "",
                .handler = hello_handler
        }
};

int main(
        void
) {
        struct req_ctx request = {
                .recv_buf = recv_buf,
                .recv_buf_size = RECV_BUF_SIZE,
                .recv_count = 0,
                .headers_buf = headers_buf,
                .headers_buf_size = HEADERS_BUF_SIZE,
                .headers_count = 0,
                .send_buf = send_buf,
                .send_buf_size = SEND_BUF_SIZE,
                .send_count = 0
        };

        IPRINTF("Starting Tusk Server\n");
        listener = init_listener(BACKLOG_SIZE);
        IPRINTF("Server launch success\n");
        signal(SIGINT, shutdown_handler);
        accept_connections(listener, process_scgi_message, routes, SIZE_OF_ARRAY(routes), &request);
        return EXIT_SUCCESS;
}
