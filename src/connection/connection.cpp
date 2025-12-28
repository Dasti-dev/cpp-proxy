#include "connection.h"

Connection::Connection(int client_fd)
    : client_fd_(client_fd),
      backend_fd_(),
      client_read_buf_(8192),
      client_write_buf_(8192),
      backend_read_buf_(8192),
      backend_write_buf_(8192),
      state_(ConnectionState::CLIENT_READING_HEADERS) {}

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

void Connection::set_backend_fd(int fd) {
    backend_fd_.reset(fd);
}

ConnectionState Connection::state() const noexcept {
    return state_;
}

void Connection::set_state(ConnectionState s) {
    state_ = s;
}

bool Connection::is_closed() const noexcept {
    return state_ == ConnectionState::CLOSED;
}

Buffer& Connection::client_read_buffer() { return client_read_buf_; }
Buffer& Connection::client_write_buffer() { return client_write_buf_; }
Buffer& Connection::backend_read_buffer() { return backend_read_buf_; }
Buffer& Connection::backend_write_buffer() { return backend_write_buf_; }
