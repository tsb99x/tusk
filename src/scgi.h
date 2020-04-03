#ifndef __SCGI_H__
#define __SCGI_H__

struct sz_pair {
        const char *key;
        const char *value;
};

struct route_binding {
        const char *path;
        const char *method;
        const char *accepts;
        size_t (*handler)(const struct sz_pair *, size_t, const char *, const char *, char *, size_t);
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

size_t respond(
        char *res_buf,
        size_t res_buf_size,
        const char *response
);

size_t process_scgi_message(
        const char *it,
        const char *it_end,
        char *res_buf,
        size_t res_buf_size,
        struct sz_pair *headers_buf,
        size_t headers_buf_size,
        struct route_binding *routes,
        size_t routes_count
);

#endif
