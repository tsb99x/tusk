#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "utility.h"

#include "scgi.h"

char digit_to_number(
        char symbol
) {
        return (symbol - '0');
}

char hex_char_to_number(
        char symbol
) {
        return (symbol - 'A') + 10;
}

const char *netstrlen(
        const char *it,
        const char *it_end,
        size_t *len
) {
        for (*len = 0; it < it_end && isdigit(*it); it++)
                *len = *len * 10 + digit_to_number(*it);
        DPRINTF("Netstring length: %zu", *len);
        return it;
}

const char *lookup_headers(
        const char *it,
        const char *it_end,
        struct headers_buf *headers
) {
        size_t pos = 0;
        while (it < it_end) {
                headers->ptr[pos].key = it;
                it += strlen(it) + 1; // skip '\0'
                if (it >= it_end) {
                        WPRINTF("Incomplete header found");
                        break;
                }
                headers->ptr[pos].value = it;
                it += strlen(it) + 1; // skip '\0'
                DPRINTF("Found header: %s = %s", headers->ptr[pos].key, headers->ptr[pos].value);
                if (++pos >= headers->size) {
                        WPRINTF("Headers buffer exhausted");
                        break;
                }
        }
        DPRINTF("Total headers count: %zu", pos);
        headers->count = pos;
        return it;
}

const char *find_header_value(
        const struct headers_buf *headers,
        const char *key
) {
        for (size_t i = 0; i < headers->count; i++)
                if (strcmp(key, headers->ptr[i].key) == 0)
                        return headers->ptr[i].value;
        return NULL;
}

// const char *const url_encode_lut[] = {
//         "%21", // !
//         "%23", // #
//         "%24", // $
//         "%25", // %
//         "%26", // &
//         "%27", // '
//         "%28", // (
//         "%29", // )
//         "%2A", // *
//         "%2B", // +
//         "%2C", // ,
//         "%2F", // /
//         "%3A", // :
//         "%3B", // ;
//         "%3D", // =
//         "%3F", // ?
//         "%40", // @
//         "%5B", // [
//         "%5D"  // ]
// };

// const char *symbols = "!#$%&'()*+,/:;=?@[]";

char from_url_encoding(
        char first,
        char second
) {
        char chr = digit_to_number(first) * 16;
        chr += isdigit(second)
                ? digit_to_number(second)
                : hex_char_to_number(second);
        DPRINTF("Decoded char '%c'", chr);
        return chr;
}

size_t url_decode(
        const char *it,
        const char *it_end,
        char *sz_buf,
        size_t sz_buf_size
) {
        size_t pos = 0;
        while (it < it_end) {
                if (*it == '%') {
                        if (it + 2 >= it_end) { // do we have 2 chars next?
                                WPRINTF("Malformed URL encoded string");
                                break;
                        }
                        char first = *(++it);
                        char second = *(++it);
                        sz_buf[pos] = from_url_encoding(first, second);
                } else {
                        sz_buf[pos] = *it;
                }
                it++;
                if (++pos >= sz_buf_size - 1) { // last position is for '\0'
                        WPRINTF("Destination buffer exhausted");
                        break;
                }
        }
        sz_buf[pos] = '\0';
        DPRINTF("Decoded string: %s", sz_buf);
        return pos;
}

size_t x_www_form_urlencoded_decode(
        const char *it,
        const char *it_end,
        char *sz_buf,
        char *sz_buf_end,
        struct sz_pair *kv_buf,
        size_t kv_buf_size
) {
        size_t pos = 0;
        while (it < it_end) {
                const char *key_start = it;
                const char *val_start = memchr(it, '=', it_end - it);
                if (val_start == NULL || ++val_start >= it_end) {
                        WPRINTF("Malformed www-form-urlencoded body");
                        return 0;
                }
                const char *key_end = val_start - 1; // rewind to '='
                it = memchr(val_start, '&', it_end - val_start);
                const char *val_end = (it == NULL) ? it_end : it - 1; // rewind to '&'

                kv_buf[pos].key = sz_buf;
                sz_buf += url_decode(key_start, key_end, sz_buf, sz_buf_end - sz_buf);
                if (++sz_buf >= sz_buf_end) { // skip '\0'
                        WPRINTF("String zero buffer exhausted");
                        break;
                }

                kv_buf[pos].value = sz_buf;
                sz_buf += url_decode(val_start, val_end, sz_buf, sz_buf_end - sz_buf);
                if (++sz_buf >= sz_buf_end) { // skip '\0'
                        WPRINTF("String zero buffer exhausted");
                        break;
                }

                DPRINTF("Found form data kv: %s = %s", kv_buf[pos].key, kv_buf[pos].value);
                if (++pos >= kv_buf_size) {
                        WPRINTF("KV buffer exhausted");
                        break;
                }
                if (it == NULL)
                        break;
                it++;
        }
        return pos;
}

void respond_sz(
        struct char_buf *send,
        const char *response
) {
        size_t len = strlen(response);
        if (len > send->size) {
                EPRINTF("Length of response string is more than buffer size");
                send->count = 0;
                return;
        }
        memcpy(send->ptr, response, len);
        send->count = len;
}

void process_scgi_message(
        struct sock_ctx *sock_ctx
) {
        struct scgi_ctx *ctx = (struct scgi_ctx *) sock_ctx;

        const char *it = ctx->recv.ptr;
        size_t headers_len;
        it = netstrlen(it, it + ctx->recv.size, &headers_len);
        if (headers_len == 0) {
                WPRINTF("No content provided in headers netstring");
                ctx->send.count = 0;
                return;
        }

        if (*it != ':') {
                WPRINTF("Request is not SCGI compliant, no ':' found");
                ctx->send.count = 0;
                return;
        }
        it++; // skip ':'

        it = lookup_headers(it, it + headers_len, &ctx->headers);
        const char *request_uri = find_header_value(&ctx->headers, "DOCUMENT_URI");
        const char *request_method = find_header_value(&ctx->headers, "REQUEST_METHOD");
        const char *content_type = find_header_value(&ctx->headers, "CONTENT_TYPE");
        // const char *content_length = find_header_value(&ctx->headers, "CONTENT_LENGTH");
        // const char *query_string = find_header_value(&ctx->headers, "QUERY_STRING");

        if (*it != ',') {
                WPRINTF("Request is not SCGI compliant, no ',' found");
                ctx->send.count = 0;
                return;
        }
        it++; // skip ','

        for (size_t i = 0; i < ctx->routes.count; i++) {
                if (strcmp(ctx->routes.ptr[i].path, request_uri) != 0)
                        continue;
                if (strcmp(ctx->routes.ptr[i].method, request_method) != 0) {
                        DPRINTF("Request method %s is not allowed", request_method);
                        respond_sz(&ctx->send,
                                "Status: 405 Method Not Allowed\r\n" \
                                "Content-Type: text/plain\r\n"       \
                                "\r\n"                               \
                                "Method Not Allowed");
                        return;
                }
                if (strcmp(ctx->routes.ptr[i].accepts, content_type) != 0) {
                        DPRINTF("Media type %s is not allowed", content_type);
                        respond_sz(&ctx->send,
                                "Status: 415 Unsupported Media Type\r\n" \
                                "Content-Type: text/plain\r\n"           \
                                "\r\n"                                   \
                                "Unsupported Media Type");
                        return;
                }
                IPRINTF("Handling %s route", request_uri);
                ctx->routes.ptr[i].handler(ctx);
                return;
        }
        DPRINTF("Route %s not found", request_uri);
        respond_sz(&ctx->send,
                "Status: 404 Not Found\r\n"    \
                "Content-Type: text/plain\r\n" \
                "\r\n"                         \
                "Not Found");
}
