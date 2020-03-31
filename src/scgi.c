#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "utility.h"

#include "scgi.h"

int get_netstring_len(
        const char **it,
        const char *const it_end
) {
        int len = 0;
        for (; *it < it_end && isdigit(**it); (*it)++)
                len = len * 10 + (**it - '0');
        DPRINTF("Netstring length: %d\n", len);
        return len;
}

int lookup_headers(
        const char *it,
        const char *const it_end,
        struct req_header *const buf,
        const int buf_size
) {
        int pos = 0;
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
        DPRINTF("Total headers count: %d\n", pos);
        return pos;
}

const char *find_header_value(
        const struct req_header *const headers,
        const int headers_count,
        const char *const key
) {
        for (int i = 0; i < headers_count; i++)
                if (strcmp(key, headers[i].name) == 0)
                        return headers[i].value;
        return NULL;
}

int process_scgi_message(
        const char *req_buf,
        const int req_len,
        char *res_buf,
        const int res_buf_size
) {
        const char *req_buf_end = req_buf + req_len;
        int netstring_len = get_netstring_len(&req_buf, req_buf_end);
        if (netstring_len == 0) {
                WPRINTF("No content provided in netstring\n");
                return 0;
        }

        req_buf++; // skip ':'

        const char *netstring_end = req_buf + netstring_len;
        struct req_header headers_buf[REQ_HEADERS_BUF_SIZE];
        int headers_count = lookup_headers(req_buf, netstring_end, headers_buf, REQ_HEADERS_BUF_SIZE);
        const char *request_method = find_header_value(headers_buf, headers_count, "REQUEST_METHOD");
        const char *request_uri = find_header_value(headers_buf, headers_count, "DOCUMENT_URI");
        const char *query_string = find_header_value(headers_buf, headers_count, "QUERY_STRING");
        const char *content_length = find_header_value(headers_buf, headers_count, "CONTENT_LENGTH");
        const char *content_type = find_header_value(headers_buf, headers_count, "CONTENT_TYPE");

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
        strcpy(res_buf, response);
        return (int) strlen(response);
}
