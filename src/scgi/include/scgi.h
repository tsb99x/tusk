#pragma once

#include <socket.h>

struct sz_pair {
        const char *key;
        const char *value;
};

struct headers_buf {
        struct sz_pair *const ptr;
        const size_t size;
        size_t count;
};

struct routes_arr {
        const struct route_binding *ptr;
        size_t count;
};

struct scgi_ctx {
        struct char_buf recv;
        struct char_buf send;
        struct headers_buf headers;
        const struct routes_arr routes;
};

struct route_binding {
        const char *path;
        const char *method;
        const char *accepts;
        void (*handler)(struct scgi_ctx *);
};

const char *netstrlen(
        const char *it,
        const char *it_end,
        size_t *len
);

const char *lookup_headers(
        const char *it,
        const char *it_end,
        struct headers_buf *headers
);

const char *find_header_value(
        const struct headers_buf *headers,
        const char *key
);

size_t url_decode(
        const char *it,
        const char *it_end,
        char *sz_buf,
        size_t sz_buf_size
);

size_t x_www_form_urlencoded_decode(
        const char *it,
        const char *it_end,
        char *sz_buf,
        char *sz_buf_end,
        struct sz_pair *kv_buf,
        size_t kv_buf_size
);

void respond_sz(
        struct char_buf *send,
        const char *response
);

void process_scgi_message(
        struct sock_ctx *sock_ctx
);
