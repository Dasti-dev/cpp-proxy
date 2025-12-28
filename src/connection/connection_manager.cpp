// (same includes and LOG macro as before)

#include "connection_manager.h"
#include "core/socket/socket.h"

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iostream>

#define LOG(msg) std::cerr << "[proxy] " << msg << std::endl;

ConnectionManager::ConnectionManager(EpollLoop& loop)
    : loop_(loop) {}

/* ================= ADD CLIENT ================= */

void ConnectionManager::add_client(int client_fd) {
    auto [it, _] = connections_.emplace(client_fd, Connection(client_fd));
    Connection* conn = &it->second;

    LOG("new client fd=" << client_fd);

    loop_.add(client_fd, EPOLLIN | EPOLLRDHUP, conn);
}

/* ================= DISPATCH ================= */

void ConnectionManager::handle_event(Connection* conn, int fd, uint32_t events) {
    if (events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) {
        LOG("epoll error/hup");
        close_connection(conn);
        return;
    }

    switch (conn->state()) {
        case ConnectionState::CLIENT_READING_HEADERS:
            if (fd == conn->client_fd() && (events & EPOLLIN))
                handle_client_read(conn);
            break;

        case ConnectionState::BACKEND_CONNECTING:
            if (fd == conn->backend_fd() && (events & EPOLLOUT))
                handle_backend_connect(conn);
            break;

        case ConnectionState::BACKEND_WRITING_REQUEST:
            if (fd == conn->backend_fd() && (events & EPOLLOUT))
                handle_backend_write(conn);
            break;

        case ConnectionState::BACKEND_READING_RESPONSE:
            if (fd == conn->backend_fd() && (events & EPOLLIN))
                handle_backend_read(conn);
            break;

        case ConnectionState::CLIENT_WRITING_RESPONSE:
            if (fd == conn->client_fd() && (events & EPOLLOUT))
                handle_client_write(conn);
            break;

        default:
            break;
    }
}

/* ================= BACKEND CONNECT ================= */

int ConnectionManager::connect_backend() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    int rc = ::connect(fd, (sockaddr*)&addr, sizeof(addr));
    if (rc < 0 && errno != EINPROGRESS) {
        ::close(fd);
        return -1;
    }
    return fd;
}

/* ================= CLIENT READ ================= */

void ConnectionManager::handle_client_read(Connection* conn) {
    Buffer& rbuf = conn->client_read_buffer();

    while (true) {
        ssize_t n = Socket::read(
            conn->client_fd(),
            rbuf.write_ptr(),
            rbuf.writable_bytes()
        );

        if (n > 0) {
            LOG("read " << n << " bytes from client");
            rbuf.commit(n);

            HttpRequestInfo info{};
            if (parser_.parse(
                    rbuf.read_ptr(),
                    rbuf.readable_bytes(),
                    info
                ) != HttpParseResult::COMPLETE)
                continue;

            LOG("HTTP request COMPLETE");

            int bfd = connect_backend();
            if (bfd < 0) {
                close_connection(conn);
                return;
            }

            conn->set_backend_fd(bfd);

            Buffer& bwbuf = conn->backend_write_buffer();
            std::memcpy(bwbuf.write_ptr(), rbuf.read_ptr(), rbuf.readable_bytes());
            bwbuf.commit(rbuf.readable_bytes());
            rbuf.clear();

            conn->set_state(ConnectionState::BACKEND_CONNECTING);
            loop_.add(bfd, EPOLLOUT | EPOLLRDHUP, conn);
            return;
        }

        if (n == 0) {
            close_connection(conn);
            return;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        close_connection(conn);
        return;
    }
}

/* ================= BACKEND CONNECT COMPLETE ================= */

void ConnectionManager::handle_backend_connect(Connection* conn) {
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(conn->backend_fd(), SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
        close_connection(conn);
        return;
    }

    LOG("backend connected successfully");

    conn->set_state(ConnectionState::BACKEND_WRITING_REQUEST);
    loop_.modify(conn->backend_fd(), EPOLLOUT | EPOLLRDHUP, conn);
}

/* ================= BACKEND WRITE ================= */

void ConnectionManager::handle_backend_write(Connection* conn) {
    Buffer& buf = conn->backend_write_buffer();

    ssize_t n = Socket::write(
        conn->backend_fd(),
        buf.read_ptr(),
        buf.readable_bytes()
    );

    if (n <= 0) {
        close_connection(conn);
        return;
    }

    LOG("wrote " << n << " bytes to backend");
    buf.consume(n);

    if (buf.readable_bytes() == 0) {
        conn->set_state(ConnectionState::BACKEND_READING_RESPONSE);
        loop_.modify(conn->backend_fd(), EPOLLIN | EPOLLRDHUP, conn);
    }
}

/* ================= BACKEND READ ================= */

void ConnectionManager::handle_backend_read(Connection* conn) {
    Buffer& rbuf = conn->backend_read_buffer();
    Buffer& cwbuf = conn->client_write_buffer();

    while (true) {
        ssize_t n = Socket::read(
            conn->backend_fd(),
            rbuf.write_ptr(),
            rbuf.writable_bytes()
        );

        if (n > 0) {
            LOG("read " << n << " bytes from backend");
            rbuf.commit(n);

            std::memcpy(cwbuf.write_ptr(), rbuf.read_ptr(), rbuf.readable_bytes());
            cwbuf.commit(rbuf.readable_bytes());
            rbuf.clear();

            conn->set_state(ConnectionState::CLIENT_WRITING_RESPONSE);
            loop_.modify(conn->client_fd(), EPOLLOUT | EPOLLRDHUP, conn);
        }
        else if (n == 0) {
            LOG("backend closed — draining client buffer");
            loop_.remove(conn->backend_fd());   // backend done
            conn->set_backend_fd(-1);
            conn->set_state(ConnectionState::CLIENT_WRITING_RESPONSE);
            return;
        }
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            close_connection(conn);
            return;
        }
    }
}

/* ================= CLIENT WRITE ================= */

void ConnectionManager::handle_client_write(Connection* conn) {
    Buffer& buf = conn->client_write_buffer();

    ssize_t n = Socket::write(
        conn->client_fd(),
        buf.read_ptr(),
        buf.readable_bytes()
    );

    if (n <= 0) {
        close_connection(conn);
        return;
    }

    LOG("wrote " << n << " bytes to client");
    buf.consume(n);

    if (buf.readable_bytes() == 0) {
        LOG("response fully sent — closing");
        close_connection(conn);
    }
}

/* ================= CLOSE ================= */

void ConnectionManager::close_connection(Connection* conn) {
    LOG("closing connection client_fd=" << conn->client_fd());

    loop_.remove(conn->client_fd());
    if (conn->backend_fd() >= 0)
        loop_.remove(conn->backend_fd());

    connections_.erase(conn->client_fd());
}
