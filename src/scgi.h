#ifndef __SCGI_H__
#define __SCGI_H__

#define REQ_HEADERS_BUF_SIZE 64

struct req_header {
    const char *name;
    const char *value;
};

int get_netstring_len(
        const char **it,
        const char *const it_end
);

int lookup_headers(
        const char *it,
        const char *const it_end,
        struct req_header *const buf,
        const int buf_size
);

const char *find_header_value(
        const struct req_header *const headers,
        const int headers_count,
        const char *const key
);

int process_scgi_message(
        const char *req_buf,
        const int req_len,
        char *res_buf,
        const int res_buf_size
);

#endif
