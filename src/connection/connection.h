#pragma once

#include "connection_state.h"

#include "core/fd/fd_wrapper.h"
#include "core/buffer/buffer.h"

/*
 * Connection
 * ----------
 * Owns client + backend sockets and buffers.
 */

class Connection {
public:
    explicit Connection(int client_fd);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&) noexcept;
    Connection& operator=(Connection&&) noexcept;

    ~Connection();

    int client_fd() const noexcept;
    int backend_fd() const noexcept;

    void set_backend_fd(int fd);

    ConnectionState state() const noexcept;
    void set_state(ConnectionState s);

    bool is_closed() const noexcept;

    Buffer& client_read_buffer();
    Buffer& client_write_buffer();

    Buffer& backend_read_buffer();
    Buffer& backend_write_buffer();

private:
    FDWrapper client_fd_;
    FDWrapper backend_fd_;

    Buffer client_read_buf_;
    Buffer client_write_buf_;

    Buffer backend_read_buf_;
    Buffer backend_write_buf_;

    ConnectionState state_;
};
