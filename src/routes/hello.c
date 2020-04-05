#include "_routes.h"

#include "../utility.h"

void hello_handler(
        struct scgi_ctx *ctx
) {
        // x_www_form_urlencoded_decode(
        //         it, it_end, 
        //         raw_sz_form_data, raw_sz_form_data + RAW_SZ_FORM_DATA_BUF_SIZE,
        //         form_data, FORM_DATA_BUF_SIZE
        // );

        respond_sz(&ctx->send,
                "Status: 200 OK\r\n"                \
                "Content-Type: text/plain\r\n"      \
                "\r\n"                              \
                "Hello from Tusk");
}
