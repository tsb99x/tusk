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
        DPRINTF("Netstring length: %zu\n", *len);
        return it;
}

size_t lookup_headers(
        const char *it,
        const char *it_end,
        struct sz_pair *buf,
        size_t buf_size
) {
        size_t pos = 0;
        while (it < it_end) {
                buf[pos].key = it;
                it += strlen(it) + 1; // skip '\0'
                if (it >= it_end) {
                        WPRINTF("Incomplete header found\n");
                        break;
                }
                buf[pos].value = it;
                it += strlen(it) + 1; // skip '\0'
                DPRINTF("Found header: %s = %s\n", buf[pos].key, buf[pos].value);
                if (++pos >= buf_size) {
                        WPRINTF("Headers buffer exhausted\n");
                        break;
                }
        }
        DPRINTF("Total headers count: %zu\n", pos);
        return pos;
}

const char *find_header_value(
        const struct sz_pair *headers,
        size_t headers_size,
        const char *key
) {
        for (size_t i = 0; i < headers_size; i++)
                if (strcmp(key, headers[i].key) == 0)
                        return headers[i].value;
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
        DPRINTF("Decoded char '%c'\n", chr);
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
                                WPRINTF("Malformed URL encoded string\n");
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
                        WPRINTF("Destination buffer exhausted\n");
                        break;
                }
        }
        sz_buf[pos] = '\0';
        DPRINTF("Decoded string: %s\n", sz_buf);
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
                        WPRINTF("Malformed www-form-urlencoded body\n");
                        return 0;
                }
                const char *key_end = val_start - 1; // rewind to '='
                it = memchr(val_start, '&', it_end - val_start);
                const char *val_end = (it == NULL) ? it_end : it - 1; // rewind to '&'

                kv_buf[pos].key = sz_buf;
                sz_buf += url_decode(key_start, key_end, sz_buf, sz_buf_end - sz_buf);
                if (++sz_buf >= sz_buf_end) { // skip '\0'
                        WPRINTF("String zero buffer exhausted\n");
                        break;
                }

                kv_buf[pos].value = sz_buf;
                sz_buf += url_decode(val_start, val_end, sz_buf, sz_buf_end - sz_buf);
                if (++sz_buf >= sz_buf_end) { // skip '\0'
                        WPRINTF("String zero buffer exhausted\n");
                        break;
                }

                DPRINTF("Found form data kv: %s = %s\n", kv_buf[pos].key, kv_buf[pos].value);
                if (++pos >= kv_buf_size) {
                        WPRINTF("KV buffer exhausted\n");
                        break;
                }
                if (it == NULL)
                        break;
                it++;
        }
        return pos;
}

size_t respond(
        char *res_buf,
        size_t res_buf_size,
        const char *response
) {
        strncpy(res_buf, response, res_buf_size - 1);
        return strlen(response);
}

size_t process_scgi_message(
        const char *it,
        const char *it_end,
        char *res_buf,
        size_t res_buf_size,
        struct route_binding *routes,
        size_t routes_count
) {
        size_t headers_len;
        it = netstrlen(it, it_end, &headers_len);
        if (headers_len == 0) {
                WPRINTF("No content provided in headers netstring\n");
                return 0;
        }

        if (*it != ':') {
                WPRINTF("Request is not SCGI compliant, no ':' found\n");
                return 0;
        }
        it++; // skip ':'

        const char *netstring_end = it + headers_len;
        size_t headers_count = lookup_headers(it, netstring_end, headers, REQ_HEADERS_BUF_SIZE);
        const char *request_uri = find_header_value(headers, headers_count, "DOCUMENT_URI");
        const char *request_method = find_header_value(headers, headers_count, "REQUEST_METHOD");
        const char *content_type = find_header_value(headers, headers_count, "CONTENT_TYPE");
        // const char *content_length = find_header_value(headers, headers_count, "CONTENT_LENGTH");
        // const char *query_string = find_header_value(headers, headers_count, "QUERY_STRING");

        if (*(it = netstring_end) != ',') {
                WPRINTF("Request is not SCGI compliant, no ',' found\n");
                return 0;
        }
        it++; // skip ','

        for (size_t i = 0; i < routes_count; i++) {
                if (strcmp(routes[i].path, request_uri) != 0)
                        continue;
                if (strcmp(routes[i].method, request_method) != 0) {
                        DPRINTF("Request method %s is not allowed\n", request_method);
                        return respond(res_buf, res_buf_size,
                                "Status: 405 Method Not Allowed\r\n" \
                                "Content-Type: text/plain\r\n"       \
                                "\r\n"                               \
                                "Method Not Allowed");
                }
                if (strcmp(routes[i].accepts, content_type) != 0) {
                        DPRINTF("Media type %s is not allowed\n", content_type);
                        return respond(res_buf, res_buf_size,
                                "Status: 415 Unsupported Media Type\r\n" \
                                "Content-Type: text/plain\r\n"           \
                                "\r\n"                                   \
                                "Unsupported Media Type");
                }
                return routes[i].handler(it, it_end, res_buf, res_buf_size);
        }
        DPRINTF("Route %s not found\n", request_uri);
        return respond(res_buf, res_buf_size,
                "Status: 404 Not Found\r\n"    \
                "Content-Type: text/plain\r\n" \
                "\r\n"                         \
                "Not Found");
}
