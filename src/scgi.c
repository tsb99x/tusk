#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "utility.h"

#include "scgi.h"

size_t netstrlen(
        const char **it,
        const char *it_end
) {
        size_t len = 0;
        for (; *it < it_end && isdigit(**it); (*it)++)
                len = len * 10 + (**it - '0');
        DPRINTF("Netstring length: %zu\n", len);
        return len;
}

size_t lookup_headers(
        const char *it,
        const char *it_end,
        struct req_header *buf,
        size_t buf_size
) {
        size_t pos = 0;
        while (it < it_end) {
                buf[pos].name = it;
                it += strlen(it) + 1; // skip '\0'
                if (it >= it_end) {
                        WPRINTF("Incomplete header found\n");
                        break;
                }
                buf[pos].value = it;
                it += strlen(it) + 1; // skip '\0'
                DPRINTF("Found header: %s = %s\n", buf[pos].name, buf[pos].value);
                if (++pos >= buf_size) {
                        WPRINTF("Headers buffer exhausted\n");
                        break;
                }
        }
        DPRINTF("Total headers count: %zu\n", pos);
        return pos;
}

const char *find_header_value(
        const struct req_header *headers,
        size_t headers_size,
        const char *key
) {
        for (size_t i = 0; i < headers_size; i++)
                if (strcmp(key, headers[i].name) == 0)
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

char from_url_encoding(
        char first,
        char second
) {
        DPRINTF("Decoding from chars '%c', '%c'\n", first, second);
        char chr = digit_to_number(first) * 16;
        chr += isdigit(second)
                ? digit_to_number(second)
                : hex_char_to_number(second);
        DPRINTF("Decoded char '%c'\n", chr);
        return chr;
}

size_t url_decode(
        const char *src,
        const char *src_end,
        char *dst_buf,
        size_t dst_buf_size
) {
        size_t pos = 0;
        while (src < src_end) {
                if (*src == '%') {
                        if (src + 2 >= src_end) {
                                DPRINTF("Malformed URL encoded string\n");
                                return 0;
                        }
                        char first = *(++src);
                        char second = *(++src);
                        dst_buf[pos] = from_url_encoding(first, second);
                } else {
                        dst_buf[pos] = *src;
                }
                src++;
                if (++pos >= dst_buf_size) {
                        WPRINTF("Destination buffer exhausted\n");
                        return pos;
                }
        }
        DPRINTF("Decoded string: %.*s\n", (int) pos, dst_buf);
        return pos;
}

size_t process_scgi_message(
        const char *req_buf,
        size_t req_size,
        char *res_buf,
        size_t res_buf_size
) {
        const char *req_buf_end = req_buf + req_size;
        size_t headers_len = netstrlen(&req_buf, req_buf_end);
        if (headers_len == 0) {
                WPRINTF("No content provided in headers netstring\n");
                return 0;
        }

        req_buf++; // skip ':'

        const char *netstring_end = req_buf + headers_len;
        struct req_header headers_buf[REQ_HEADERS_BUF_SIZE];
        size_t headers_count = lookup_headers(req_buf, netstring_end, headers_buf, REQ_HEADERS_BUF_SIZE);
        // const char *request_method = find_header_value(headers_buf, headers_count, "REQUEST_METHOD");
        // const char *request_uri = find_header_value(headers_buf, headers_count, "DOCUMENT_URI");
        // const char *query_string = find_header_value(headers_buf, headers_count, "QUERY_STRING");
        const char *content_length = find_header_value(headers_buf, headers_count, "CONTENT_LENGTH");
        // const char *content_type = find_header_value(headers_buf, headers_count, "CONTENT_TYPE");

        req_buf = ++netstring_end; // skip ','

        int content_length_val = atoi(content_length);
        DPRINTF("Request Content-Length is %d\n", content_length_val);
        if (req_buf >= req_buf_end)
                DPRINTF("No content provided\n");
        else
                DPRINTF("Content: %.*s\n", content_length_val, req_buf);

        const char *response = "Status: 200 OK\r\n" \
                "Content-Type: text/plain\r\n"      \
                "\r\n"                              \
                "Hello from Tusk";
        strncpy(res_buf, response, res_buf_size - 1);
        return strlen(response);
}
