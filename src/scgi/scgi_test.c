#include <scgi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        NAME_TEST();

        const char *str;
        size_t len;

        str = "11:Hello, Test!,";
        str = netstrlen(str, str + strlen(str), &len);
        REQUIRE(len == 11);
        REQUIRE(*str == ':');

        str = "Non-netstring string";
        str = netstrlen(str, str + strlen(str), &len);
        REQUIRE(len == 0);
        REQUIRE(*str == 'N');
}

#define HEADERS_BUF_SIZE 2

void test_lookup_headers(
        void
) {
        NAME_TEST();

        const char *str, *res;
        size_t len;
        struct sz_pair sz_pair_buf[HEADERS_BUF_SIZE];
        struct headers_buf buf = {
                .ptr = sz_pair_buf,
                .size = HEADERS_BUF_SIZE
        };

        str = "CONTENT_LENGTH" "\0" "0" "\0";
        len = 17;
        res = lookup_headers(str, str + len, &buf);
        REQUIRE(res == str + len);
        REQUIRE(buf.count == 1);
        REQUIRE(!strcmp(buf.ptr[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf.ptr[0].value, "0"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0" "1" "\0";
        len = 24;
        res = lookup_headers(str, str + len, &buf);
        REQUIRE(res == str + len);
        REQUIRE(buf.count == 2);
        REQUIRE(!strcmp(buf.ptr[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf.ptr[0].value, "0"));
        REQUIRE(!strcmp(buf.ptr[1].key, "SCGI"));
        REQUIRE(!strcmp(buf.ptr[1].value, "1"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0";
        len = 22;
        res = lookup_headers(str, str + len, &buf);
        REQUIRE(res == str + len);
        REQUIRE(buf.count == 1);
        REQUIRE(!strcmp(buf.ptr[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf.ptr[0].value, "0"));

        str = "CONTENT_LENGTH" "\0" "0" "\0" \
                "SCGI" "\0" "1" "\0" \
                "UNREACHABLE" "\0" "1" "\0";
        len = 38;
        res = lookup_headers(str, str + len, &buf);
        REQUIRE(res == str + 24);
        REQUIRE(buf.count == 2);
        REQUIRE(!strcmp(buf.ptr[0].key, "CONTENT_LENGTH"));
        REQUIRE(!strcmp(buf.ptr[0].value, "0"));
        REQUIRE(!strcmp(buf.ptr[1].key, "SCGI"));
        REQUIRE(!strcmp(buf.ptr[1].value, "1"));
}

void test_find_header_value(
        void
) {
        NAME_TEST();

        const char *res;
        struct sz_pair sz_pair_arr[] = {
                {.key = "CONTENT_LENGTH", .value = "0"},
                {.key = "SCGI", .value = "1"}
        };
        struct headers_buf headers = {
                .ptr = sz_pair_arr,
                .count = 2
        };

        res = find_header_value(&headers, "CONTENT_LENGTH");
        REQUIRE(res == headers.ptr[0].value);

        res = find_header_value(&headers, "NON-EXISTING");
        REQUIRE(res == NULL);
}

#define URL_BUF_SIZE 64

void test_url_decode(
        void
) {
        NAME_TEST();

        const char *original = "Hello, Tusk!";
        const char *encoded = "Hello%2C%20Tusk%21";
        char buf[URL_BUF_SIZE];
        size_t len;

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
