#include "connection.h"

Connection::Connection(int client_fd)
    : client_fd_(client_fd),
      backend_fd_(),
      client_read_buf_(4096),
      client_write_buf_(4096),
      backend_read_buf_(4096),
      backend_write_buf_(4096),
      state_(ConnectionState::INIT) {}

Connection::Connection(Connection&& other) noexcept
    : client_fd_(std::move(other.client_fd_)),
      backend_fd_(std::move(other.backend_fd_)),
      client_read_buf_(std::move(other.client_read_buf_)),
      client_write_buf_(std::move(other.client_write_buf_)),
      backend_read_buf_(std::move(other.backend_read_buf_)),
      backend_write_buf_(std::move(other.backend_write_buf_)),
      state_(other.state_) {
    other.state_ = ConnectionState::CLOSED;
}

Connection& Connection::operator=(Connection&& other) noexcept {
    if (this != &other) {
        client_fd_ = std::move(other.client_fd_);
        backend_fd_ = std::move(other.backend_fd_);
        client_read_buf_ = std::move(other.client_read_buf_);
        client_write_buf_ = std::move(other.client_write_buf_);
        backend_read_buf_ = std::move(other.backend_read_buf_);
        backend_write_buf_ = std::move(other.backend_write_buf_);
        state_ = other.state_;

        other.state_ = ConnectionState::CLOSED;
    }
    return *this;
}

Connection::~Connection() {
    state_ = ConnectionState::CLOSED;
}

int Connection::client_fd() const noexcept {
    return client_fd_.get();
}

int Connection::backend_fd() const noexcept {
    return backend_fd_.get();
}

ConnectionState Connection::state() const noexcept {
    return state_;
}

void Connection::set_state(ConnectionState new_state) {
    // No validation yet — will be enforced later
    state_ = new_state;
}

bool Connection::is_closed() const noexcept {
    return state_ == ConnectionState::CLOSED;
}

Buffer& Connection::client_read_buffer() {
    return client_read_buf_;
}

Buffer& Connection::client_write_buffer() {
    return client_write_buf_;
}

Buffer& Connection::backend_read_buffer() {
    return backend_read_buf_;
}

Buffer& Connection::backend_write_buffer() {
    return backend_write_buf_;
}
