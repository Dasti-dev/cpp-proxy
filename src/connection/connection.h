#pragma once
#include <memory>
#include <iostream>

#include "core/buffer/buffer.h"
#include "core/fd/fd_wrapper.h"
#include "connection_state.h"

struct Connection {
    struct EpollTag {
        Connection* conn;
        bool is_client;
    };

    FDWrapper client_fd_;
    FDWrapper backend_fd_;

    Buffer client_read_buf{4096};
    Buffer client_write_buf{8192};
    Buffer backend_read_buf{8192};

    ConnectionState state_{ConnectionState::READING_REQUEST};
    bool closing_{false};

    EpollTag client_tag{this, true};
    EpollTag backend_tag{this, false};

    explicit Connection(int cfd)
        : client_fd_(cfd) {
        std::cout << "[conn] created, client_fd=" << cfd
                  << " state=READING_REQUEST\n";
    }

    int client_fd() const { return client_fd_.get(); }
    int backend_fd() const { return backend_fd_.get(); }

    void set_backend_fd(int fd) {
        backend_fd_.reset(fd);
    }

    bool is_closing() const { return closing_; }
    void mark_closing() { closing_ = true; }
};
