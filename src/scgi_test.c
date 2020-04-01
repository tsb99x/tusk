#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scgi.h"

#define REQUIRE(predicate)                                                      \
        do {                                                                    \
                if (!(predicate)) {                                             \
                        fprintf(stderr, "Assertion failed '" #predicate "'\n"); \
                        exit(EXIT_FAILURE);                                     \
                }                                                               \
        } while (0)

#define NAME_TEST() \
        printf("\n*** %s ***\n\n", __func__)

void test_get_netstring_size(
        void
) {
        const char *str;
        size_t length;

        NAME_TEST();

        str = "11:Hello, Test!,";
        length = netstrlen(&str, str + strlen(str));
        REQUIRE(length == 11);
        REQUIRE(*str == ':');

        str = "Non-netstring string";
        length = netstrlen(&str, str + strlen(str));
        REQUIRE(length == 0);
        REQUIRE(*str == 'N');
}

#define HEADERS_BUF_SIZE 2

void test_lookup_headers(
        void
) {
        struct sz_pair buf[HEADERS_BUF_SIZE];

        char *str;
        size_t len, headers_count;

        NAME_TEST();

        str = "CONTENT_LENGTH" "\0" "0" "\0";
        len = 17;
        headers_count = lookup_headers(str, str + len, buf, HEADERS_BUF_SIZE);
        REQUIRE(headers_count == 1);
        REQUIRE(!strcmp(buf[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0" "1" "\0";
        len = 24;
        headers_count = lookup_headers(str, str + len, buf, HEADERS_BUF_SIZE);
        REQUIRE(headers_count == 2);
        REQUIRE(!strcmp(buf[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));
        REQUIRE(!strcmp(buf[1].key, "SCGI"));
        REQUIRE(!strcmp(buf[1].value, "1"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0";
        len = 22;
        headers_count = lookup_headers(str, str + len, buf, HEADERS_BUF_SIZE);
        REQUIRE(headers_count == 1);
        REQUIRE(!strcmp(buf[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0" "1" "\0" \
                "UNREACHABLE" "\0" "1" "\0";
        len = 38;
        headers_count = lookup_headers(str, str + len, buf, HEADERS_BUF_SIZE);
        REQUIRE(headers_count == 2);
        REQUIRE(!strcmp(buf[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf[0].value, "0"));
        REQUIRE(!strcmp(buf[1].key, "SCGI"));
        REQUIRE(!strcmp(buf[1].value, "1"));
}

void test_find_header_value(
        void
) {
        struct sz_pair headers[] = {
                {.key = "CONTENT_LENGTH", .value = "0"},
                {.key = "SCGI", .value = "1"}
        };

        NAME_TEST();

        const char *res;

        res = find_header_value(headers, 2, "CONTENT_LENGTH");
        REQUIRE(res == headers[0].value);

        res = find_header_value(headers, 2, "NON-EXISTING");
        REQUIRE(res == NULL);
}

#define URL_BUF_SIZE 64

void test_url_decode(
        void
) {
        const char *original = "Hello, Tusk!";
        const char *encoded = "Hello%2C%20Tusk%21";
        char buf[URL_BUF_SIZE];
        size_t len;

        NAME_TEST();

        encoded = "Hello%2C%20Tusk%21";
        len = url_decode(encoded, encoded + strlen(encoded), buf, URL_BUF_SIZE);
        REQUIRE(len == 12);
        REQUIRE(!strcmp(original, buf));

        encoded = "Hello%2C%20Tusk%2";
        len = url_decode(encoded, encoded + strlen(encoded), buf, URL_BUF_SIZE);
        REQUIRE(len == 11);
        REQUIRE(!strcmp("Hello, Tusk", buf));
}

void test_x_www_form_urlencoded_decode(
        void
) {
        const char *str = "a%2Cb%2Cc=1%202%203%21";
        char sz_buf[URL_BUF_SIZE];
        size_t len;
        struct sz_pair key_val_buf[URL_BUF_SIZE];

        NAME_TEST();

        len = x_www_form_urlencoded_decode(str, str + strlen(str), sz_buf, sz_buf + URL_BUF_SIZE, key_val_buf, URL_BUF_SIZE);
        REQUIRE(len == 1);
        REQUIRE(!strcmp(key_val_buf[0].key, "a,b,c"));
        REQUIRE(!strcmp(key_val_buf[0].value, "1 2 3!"));
}

int main(
        void
) {
        test_get_netstring_size();
        test_lookup_headers();
        test_find_header_value();
        test_url_decode();
        test_x_www_form_urlencoded_decode();
        return EXIT_SUCCESS;
}
