#pragma once

#include <scgi.h>

void hello_handler(
        struct scgi_ctx *ctx
);

void login_handler(
        struct scgi_ctx *ctx
);
