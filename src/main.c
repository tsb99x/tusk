#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "socket.h"

SOCKET listener;

void shutdown_handler(
        int signum
) {
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
        accept_connections(listener);
        return EXIT_SUCCESS;
}
