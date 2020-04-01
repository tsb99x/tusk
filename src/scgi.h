#ifndef __SCGI_H__
#define __SCGI_H__

#define REQ_HEADERS_BUF_SIZE 64

struct req_header {
    const char *name;
    const char *value;
};

size_t netstrlen(
        const char **it,
        const char *it_end
);

size_t lookup_headers(
        const char *it,
        const char *it_end,
        struct req_header *buf,
        size_t buf_size
);

const char *find_header_value(
        const struct req_header *headers,
        size_t headers_size,
        const char *key
);

size_t url_decode(
        const char *src,
        const char *src_end,
        char *dst_buf,
        size_t dst_buf_size
);

size_t process_scgi_message(
        const char *req_buf,
        size_t req_size,
        char *res_buf,
        size_t res_buf_size
);

#endif
