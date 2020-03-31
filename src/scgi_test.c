#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scgi.h"

#define BUF_SIZE 2

#define REQUIRE(predicate)                                                      \
        do {                                                                    \
                if (!(predicate)) {                                             \
                        fprintf(stderr, "Assertion failed '" #predicate "'\n"); \
                        exit(EXIT_FAILURE);                                     \
                }                                                               \
        } while (0)

void test_get_netstring_size(
        void
) {
        char *str;
        int length;

        str = "11:Hello, Test!,";
        length = get_netstring_len(&str, str + strlen(str));
        REQUIRE(length == 11);
        REQUIRE(*str == ':');

        str = "Non-netstring string";
        length = get_netstring_len(&str, str + strlen(str));
        REQUIRE(length == 0);
        REQUIRE(*str == 'N');
}

void test_lookup_headers(
        void
) {
        struct req_header buf[BUF_SIZE];

        char *str;
        int len, headers_count;

        str = "CONTENT_LENGTH" "\0" "0" "\0";
        len = 17;
        headers_count = lookup_headers(str, str + len, buf, BUF_SIZE);
        REQUIRE(headers_count == 1);
        REQUIRE(!strcmp(buf[0].name, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0" "1" "\0";
        len = 24;
        headers_count = lookup_headers(str, str + len, buf, BUF_SIZE);
        REQUIRE(headers_count == 2);
        REQUIRE(!strcmp(buf[0].name, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));
        REQUIRE(!strcmp(buf[1].name, "SCGI"));
        REQUIRE(!strcmp(buf[1].value, "1"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0";
        len = 22;
        headers_count = lookup_headers(str, str + len, buf, BUF_SIZE);
        REQUIRE(headers_count == 1);
        REQUIRE(!strcmp(buf[0].name, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0" "1" "\0" \
                "UNREACHABLE" "\0" "1" "\0";
        len = 38;
        headers_count = lookup_headers(str, str + len, buf, BUF_SIZE);
        REQUIRE(headers_count == 2);
        REQUIRE(!strcmp(buf[0].name, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));
        REQUIRE(!strcmp(buf[1].name, "SCGI"));
        REQUIRE(!strcmp(buf[1].value, "1"));
}

void test_find_header_value(
        void
) {
        struct req_header headers[] = {
                {.name = "CONTENT_LENGTH", .value = "0"},
                {.name = "SCGI", .value = "1"}
        };

        const char *res;

        res = find_header_value(headers, 2, "CONTENT_LENGTH");
        REQUIRE(res == headers[0].value);

        res = find_header_value(headers, 2, "NON-EXISTING");
        REQUIRE(res == NULL);
}

int main(
        void
) {
        test_get_netstring_size();
        test_lookup_headers();
        test_find_header_value();
        return EXIT_SUCCESS;
}
