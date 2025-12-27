#include <cassert>
#include <cstring>
#include <iostream>

#include "protocol/http/http_parser.h"

/*
 * Simple unit tests for HttpParser framing logic.
 * No sockets, no buffers, no epoll.
 */

void test_incomplete_headers() {
    HttpParser parser;
    HttpRequestInfo info{};

    const char* req = "GET / HTTP/1.1\r\nHost: example.com\r\n";
    auto res = parser.parse(req, std::strlen(req), info);

    assert(res == HttpParseResult::INCOMPLETE);
}

void test_complete_headers_no_body() {
    HttpParser parser;
    HttpRequestInfo info{};

    const char* req =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    auto res = parser.parse(req, std::strlen(req), info);

    assert(res == HttpParseResult::COMPLETE);
    assert(info.header_bytes == std::strlen(req));
    assert(info.body_bytes == 0);
}

void test_headers_with_content_length_incomplete_body() {
    HttpParser parser;
    HttpRequestInfo info{};

    const char* req =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "12345";

    auto res = parser.parse(req, std::strlen(req), info);

    assert(res == HttpParseResult::INCOMPLETE);
    assert(info.header_bytes > 0);
    assert(info.body_bytes == 10);
}

void test_headers_with_content_length_complete_body() {
    HttpParser parser;
    HttpRequestInfo info{};

    const char* req =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "abcde";

    auto res = parser.parse(req, std::strlen(req), info);

    assert(res == HttpParseResult::COMPLETE);
    assert(info.body_bytes == 5);
}

void test_multiple_headers_case_insensitive() {
    HttpParser parser;
    HttpRequestInfo info{};

    const char* req =
        "POST /x HTTP/1.1\r\n"
        "host: example.com\r\n"
        "content-length: 3\r\n"
        "\r\n"
        "xyz";

    auto res = parser.parse(req, std::strlen(req), info);

    assert(res == HttpParseResult::COMPLETE);
    assert(info.body_bytes == 3);
}

int main() {
    test_incomplete_headers();
    test_complete_headers_no_body();
    test_headers_with_content_length_incomplete_body();
    test_headers_with_content_length_complete_body();
    test_multiple_headers_case_insensitive();

    std::cout << "HTTP parser tests PASSED\n";
    return 0;
}
