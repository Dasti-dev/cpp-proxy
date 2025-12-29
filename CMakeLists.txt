cmake_minimum_required(VERSION 3.16)
project(cpp_proxy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------
# Build options
# ----------------------------
option(PROXY_DEBUG "Enable proxy debug logs" OFF)

if (PROXY_DEBUG)
    add_compile_definitions(PROXY_DEBUG)
endif()

# ----------------------------
# Include paths
# ----------------------------
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# ----------------------------
# Source files
# ----------------------------
set(CORE_SOURCES
    src/core/fd/fd_wrapper.cpp
    src/core/buffer/buffer.cpp
    src/core/socket/socket.cpp
    src/core/socket/acceptor.cpp
    src/core/event_loop/epoll_loop.cpp
)

set(CONNECTION_SOURCES
    src/connection/connection.cpp
    src/connection/connection_manager.cpp
)

set(PROTOCOL_SOURCES
    src/protocol/http/http_parser.cpp
)

# ----------------------------
# Executable: proxy server
# ----------------------------
add_executable(echo_cm
    examples/echo_with_connection_manager.cpp
    ${CORE_SOURCES}
    ${CONNECTION_SOURCES}
    ${PROTOCOL_SOURCES}
)

target_link_libraries(echo_cm PRIVATE pthread)

# ----------------------------
# Unit test: HTTP parser
# ----------------------------
add_executable(http_parser_test
    tests/unit/http_parser_test.cpp
    src/protocol/http/http_parser.cpp
)

target_link_libraries(http_parser_test PRIVATE pthread)
