#ifndef __SCGI_H__
#define __SCGI_H__

struct sz_pair {
        const char *key;
        const char *value;
};

#define REQ_HEADERS_BUF_SIZE 64
struct sz_pair headers[REQ_HEADERS_BUF_SIZE];

#define RAW_SZ_FORM_DATA_BUF_SIZE 4096
char raw_sz_form_data[RAW_SZ_FORM_DATA_BUF_SIZE];

#define FORM_DATA_BUF_SIZE 64
struct sz_pair form_data[FORM_DATA_BUF_SIZE];

size_t netstrlen(
        const char **it,
        const char *it_end
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

size_t process_scgi_message(
        const char *it,
        const char *it_end,
        char *res_buf,
        size_t res_buf_size
);

#endif
