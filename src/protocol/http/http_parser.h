#pragma once

#include <cstddef>
#include <string>

/*
 * HttpParseResult
 * ----------------
 * Result of attempting to parse HTTP request framing.
 */
enum class HttpParseResult {
    INCOMPLETE,        // Need more data
    COMPLETE,          // Full request received
    ERROR              // Malformed request
};

/*
 * HttpRequestInfo
 * ----------------
 * Minimal framing information extracted from HTTP request.
 *
 * This is NOT a full HTTP representation.
 */
struct HttpRequestInfo {
    size_t header_bytes = 0;      // Bytes covering headers (\r\n\r\n included)
    size_t body_bytes = 0;        // Expected body length (Content-Length)
};

/*
 * HttpParser
 * ----------
 * Stateless framing parser for HTTP/1.x requests.
 *
 * Responsibilities:
 * - Detect end of headers
 * - Extract Content-Length if present
 * - Decide when request is complete
 *
 * Non-responsibilities:
 * - Socket I/O
 * - Buffer ownership
 * - HTTP routing
 * - Header normalization
 */
class HttpParser {
public:
    HttpParser() = default;

    // Attempt to parse framing from raw data
    HttpParseResult parse(
        const char* data,
        size_t len,
        HttpRequestInfo& out
    );

private:
    // Helper: find end of headers
    static bool find_header_end(
        const char* data,
        size_t len,
        size_t& header_end
    );

    // Helper: extract Content-Length
    static bool parse_content_length(
        const char* headers,
        size_t header_len,
        size_t& content_length
    );
};
