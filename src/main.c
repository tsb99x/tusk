#include <stdio.h>
#include <stdlib.h>

#include "socket.h"

int main(
        void
) {
        puts("Starting Tusk Server");
        SOCKET listener = init_listener();
        puts("Server launch success");
        close_listener(listener);
        return EXIT_SUCCESS;
}
