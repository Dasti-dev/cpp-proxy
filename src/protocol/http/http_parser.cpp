#include "http_parser.h"

#include <cstring>
#include <cctype>

/*
 * Look for "\r\n\r\n"
 */
bool HttpParser::find_header_end(
    const char* data,
    size_t len,
    size_t& header_end
) {
    if (len < 4) {
        return false;
    }

    for (size_t i = 0; i <= len - 4; ++i) {
        if (data[i] == '\r' &&
            data[i + 1] == '\n' &&
            data[i + 2] == '\r' &&
            data[i + 3] == '\n') {
            header_end = i + 4;
            return true;
        }
    }
    return false;
}

/*
 * Extract Content-Length header (if present)
 */
bool HttpParser::parse_content_length(
    const char* headers,
    size_t header_len,
    size_t& content_length
) {
    content_length = 0;

    const char* p = headers;
    const char* end = headers + header_len;

    while (p < end) {
        const char* line_end = static_cast<const char*>(
            std::memchr(p, '\n', end - p)
        );
        if (!line_end) {
            break;
        }

        // Case-insensitive compare for "Content-Length:"
        const char* key = "Content-Length:";
        size_t key_len = std::strlen(key);

        if (static_cast<size_t>(line_end - p) >= key_len &&
            strncasecmp(p, key, key_len) == 0) {

            const char* value = p + key_len;
            while (value < line_end && std::isspace(*value)) {
                ++value;
            }

            size_t len = 0;
            while (value < line_end && std::isdigit(*value)) {
                len = len * 10 + (*value - '0');
                ++value;
            }

            content_length = len;
            return true;
        }

        p = line_end + 1;
    }

    return false;
}

HttpParseResult HttpParser::parse(
    const char* data,
    size_t len,
    HttpRequestInfo& out
) {
    size_t header_end = 0;

    if (!find_header_end(data, len, header_end)) {
        return HttpParseResult::INCOMPLETE;
    }

    out.header_bytes = header_end;

    size_t content_length = 0;
    parse_content_length(data, header_end, content_length);
    out.body_bytes = content_length;

    // Check if full body is present
    if (len < header_end + content_length) {
        return HttpParseResult::INCOMPLETE;
    }

    return HttpParseResult::COMPLETE;
}
