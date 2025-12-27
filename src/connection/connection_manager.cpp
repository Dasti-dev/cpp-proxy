#include "connection_manager.h"

#include "core/socket/socket.h"

#include <sys/epoll.h>
#include <errno.h>
#include <cstring>

/*
 * ConnectionManager — ECHO MODE
 *
 * IMPORTANT:
 * Echo semantics = Read → Write → Close
 * This avoids FD leaks with short-lived clients (nc, ab).
 */

ConnectionManager::ConnectionManager(EpollLoop& loop)
    : loop_(loop) {}

void ConnectionManager::add_client(int client_fd) {
    Connection conn(client_fd);
    conn.set_state(ConnectionState::CLIENT_READING);

    connections_.emplace(client_fd, std::move(conn));

    loop_.add(
        client_fd,
        EPOLLIN | EPOLLRDHUP,
        reinterpret_cast<void*>(static_cast<intptr_t>(client_fd))
    );
}

void ConnectionManager::handle_event(int fd, uint32_t events) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
        return;
    }

    Connection& conn = it->second;

    if (events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
        close_connection(fd);
        return;
    }

    switch (conn.state()) {
        case ConnectionState::CLIENT_READING:
            if (events & EPOLLIN) {
                handle_client_reading(conn);
            }
            break;

        case ConnectionState::CLIENT_WRITING:
            if (events & EPOLLOUT) {
                handle_client_writing(conn);
            }
            break;

        default:
            break;
    }
}

void ConnectionManager::close_connection(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
        return;
    }

    loop_.remove(fd);
    connections_.erase(it);   // FD closed by RAII
}

/* =========================
 * Echo: READ
 * ========================= */
void ConnectionManager::handle_client_reading(Connection& conn) {
    int fd = conn.client_fd();
    Buffer& rbuf = conn.client_read_buffer();
    Buffer& wbuf = conn.client_write_buffer();

    char* wptr = rbuf.write_ptr();
    size_t cap = rbuf.writable_bytes();

    ssize_t nread = Socket::read(fd, wptr, cap);

    if (nread > 0) {
        rbuf.commit(nread);

        size_t bytes = rbuf.readable_bytes();
        std::memcpy(wbuf.write_ptr(), rbuf.read_ptr(), bytes);

        wbuf.commit(bytes);
        rbuf.clear();

        conn.set_state(ConnectionState::CLIENT_WRITING);

        loop_.modify(
            fd,
            EPOLLOUT | EPOLLRDHUP,
            reinterpret_cast<void*>(static_cast<intptr_t>(fd))
        );
    }
    else {
        close_connection(fd);
    }
}

/* =========================
 * Echo: WRITE (CLOSE AFTER)
 * ========================= */
void ConnectionManager::handle_client_writing(Connection& conn) {
    int fd = conn.client_fd();
    Buffer& wbuf = conn.client_write_buffer();

    while (wbuf.readable_bytes() > 0) {
        ssize_t nwritten = Socket::write(
            fd,
            wbuf.read_ptr(),
            wbuf.readable_bytes()
        );

        if (nwritten > 0) {
            wbuf.consume(nwritten);
        } else {
            close_connection(fd);
            return;
        }
    }

    // 🔴 ECHO SEMANTICS: CLOSE AFTER WRITE
    close_connection(fd);
}
