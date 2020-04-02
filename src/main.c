#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "utility.h"
#include "socket.h"
#include "scgi.h"

SOCKET listener;

void shutdown_handler(
        int signum
) {
        UNUSED(signum);
        puts("Shutdown in progress");
        close_listener(listener);
}

size_t hello_handler(
        const char *req,
        const char *req_end,
        char *res_buf,
        size_t res_buf_size
) {
        UNUSED(req);
        UNUSED(req_end);

        // if (!strcmp(content_type, "application/x-www-form-urlencoded")) {
        //         x_www_form_urlencoded_decode(
        //                 it, it_end, 
        //                 raw_sz_form_data, raw_sz_form_data + RAW_SZ_FORM_DATA_BUF_SIZE,
        //                 form_data, FORM_DATA_BUF_SIZE
        //         );
        // }

        return respond(res_buf, res_buf_size,
                "Status: 200 OK\r\n"                \
                "Content-Type: text/plain\r\n"      \
                "\r\n"                              \
                "Hello from Tusk");
}

struct route_binding routes[] = {
        {.path = "/hello", .method = "GET", .accepts = "", .handler = hello_handler}
};

int main(
        void
) {
        puts("Starting Tusk Server");
        listener = init_listener();
        puts("Server launch success");
        signal(SIGINT, shutdown_handler);
        accept_connections(listener, process_scgi_message, recv_buf, RECV_BUF_SIZE, send_buf, SEND_BUF_SIZE, routes, SIZE_OF_ARRAY(routes));
        return EXIT_SUCCESS;
}
