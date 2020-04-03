#ifndef __SCGI_H__
#define __SCGI_H__

struct sz_pair {
        const char *key;
        const char *value;
};

struct req_ctx {
        char *const recv_buf;
        const size_t recv_buf_size;
        size_t recv_count;

        char *const send_buf;
        const size_t send_buf_size;
        size_t send_count;

        struct sz_pair *const headers_buf;
        const size_t headers_buf_size;
        size_t headers_count;
};

struct route_binding {
        const char *path;
        const char *method;
        const char *accepts;
        void (*handler)(struct req_ctx *);
};

const char *netstrlen(
        const char *it,
        const char *it_end,
        size_t *len
);

size_t lookup_headers(
        const char *it,
        const char *it_end,
        struct sz_pair *buf,
        size_t buf_size
);

const char *find_header_value(
        const struct sz_pair *headers,
        size_t headers_size,
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
        struct req_ctx *request,
        const char *response
);

void process_scgi_message(
        struct req_ctx *request,
        const struct route_binding *routes,
        size_t routes_count
);

#endif
