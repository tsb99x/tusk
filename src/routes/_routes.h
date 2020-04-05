#ifndef __ROUTES_H__
#define __ROUTES_H__

#include "../scgi.h"

void hello_handler(
        struct scgi_ctx *ctx
);

void login_handler(
        struct scgi_ctx *ctx
);

#endif
