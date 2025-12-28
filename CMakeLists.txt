cmake_minimum_required(VERSION 3.10)
project(cpp_proxy LANGUAGES CXX)

# ===============================
# Global config
# ===============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Root include directory
include_directories(${PROJECT_SOURCE_DIR}/src)

# ===============================
# Echo HTTP server (FD-safe, HTTP-enabled)
# ===============================
add_executable(echo_cm
    examples/echo_with_connection_manager.cpp

    # Connection layer
    src/connection/connection.cpp
    src/connection/connection_manager.cpp

    # HTTP protocol
    src/protocol/http/http_parser.cpp

    # Core layers (Phase 1)
    src/core/fd/fd_wrapper.cpp
    src/core/buffer/buffer.cpp
    src/core/socket/socket.cpp
    src/core/socket/acceptor.cpp
    src/core/event_loop/epoll_loop.cpp
)

target_link_libraries(echo_cm PRIVATE pthread)

# ===============================
# HTTP parser unit tests
# ===============================
add_executable(http_parser_test
    tests/unit/http_parser_test.cpp
    src/protocol/http/http_parser.cpp
)

target_link_libraries(http_parser_test PRIVATE pthread)
