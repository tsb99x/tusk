#include <routes.h>
#include <utility.h>

#include <signal.h>
#include <stdlib.h>

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

struct route_binding routes_arr[] = {
        {
                .path = "/hello",
                .method = "GET",
                .accepts = "",
                .handler = hello_handler
        },
        {
                .path = "/login",
                .method = "GET",
                .accepts = "",
                .handler = login_handler
        }
};

struct scgi_ctx req_ctx = {
        .recv = {
                .ptr = recv_buf,
                .size = RECV_BUF_SIZE
        },
        .send = {
                .ptr = send_buf,
                .size = SEND_BUF_SIZE
        },
        .headers = {
                .ptr = headers_buf,
                .size = HEADERS_BUF_SIZE
        },
        .routes = {
                .ptr = routes_arr,
                .count = SIZE_OF_ARRAY(routes_arr)
        }
};

int main(
        void
) {
        IPRINTF("Starting Tusk Server");
        listener = init_listener(BACKLOG_SIZE);
        IPRINTF("Server launch success");
        signal(SIGINT, shutdown_handler);
        accept_connections(listener, process_scgi_message, (struct sock_ctx *) &req_ctx);
        return EXIT_SUCCESS;
}
