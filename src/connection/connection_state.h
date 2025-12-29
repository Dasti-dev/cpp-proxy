#pragma once

enum class ConnectionState {
    READING_REQUEST = 0,
    CONNECTING_BACKEND,
    READING_BACKEND,
    WRITING_CLIENT,
    CLOSING
};
