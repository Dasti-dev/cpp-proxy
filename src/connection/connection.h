#pragma once

#include "connection_state.h"

#include "core/fd/fd_wrapper.h"
#include "core/buffer/buffer.h"

/*
 * Connection
 * ----------
 * Represents a single end-to-end proxy connection.
 *
 * Owns:
 * - Client FD
 * - Backend FD (optional, later)
 * - Read / write buffers
 * - Explicit connection state
 *
 * Rules:
 * - Owns all its resources
 * - No implicit state transitions
 * - No blocking calls
 * - No exit()
 */

class Connection {
public:
    // Construct with accepted client fd
    explicit Connection(int client_fd);

    // Non-copyable
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    // Movable
    Connection(Connection&&) noexcept;
    Connection& operator=(Connection&&) noexcept;

    ~Connection();

    // Accessors
    int client_fd() const noexcept;
    int backend_fd() const noexcept;

    ConnectionState state() const noexcept;

    // Explicit state transition
    void set_state(ConnectionState new_state);

    // State helpers (read-only)
    bool is_closed() const noexcept;

    // Buffers (exposed intentionally for now)
    Buffer& client_read_buffer();
    Buffer& client_write_buffer();

    Buffer& backend_read_buffer();
    Buffer& backend_write_buffer();

private:
    FDWrapper client_fd_;
    FDWrapper backend_fd_;   // invalid until backend connected

    Buffer client_read_buf_;
    Buffer client_write_buf_;

    Buffer backend_read_buf_;
    Buffer backend_write_buf_;

    ConnectionState state_;
};
