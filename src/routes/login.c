#include <string.h>

#include "../utility.h"
#include "login.h"

#include "_routes.h"

char *append(
        char *dst,
        size_t src_len,
        const char *src
) {
        memcpy(dst, src, src_len);
        return dst + src_len;
}

void login_handler(
        struct scgi_ctx *ctx
) {
        char *it = ctx->send.ptr;

        it = append(it, 68,
                "Status: 200 OK\r\n"          \
                "Content-Type: text/html\r\n" \
                "Set-Cookie: csrf=random\r\n" \
                "\r\n");

        struct login_params params = {
                .csrf = "random"
        };
        it += login_template(&params, it);

        ctx->send.count = it - ctx->send.ptr;
}
