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

int main(
        void
) {
        puts("Starting Tusk Server");
        listener = init_listener();
        puts("Server launch success");
        signal(SIGINT, shutdown_handler);
        accept_connections(listener, process_scgi_message, recv_buf, RECV_BUF_SIZE, send_buf, SEND_BUF_SIZE);
        return EXIT_SUCCESS;
}
