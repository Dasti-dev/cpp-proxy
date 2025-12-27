#include "acceptor.h"
#include "socket.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

Acceptor::Acceptor()
    : listen_fd_(-1) {}

Acceptor::~Acceptor() {
    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
    }
}

bool Acceptor::listen(uint16_t port, int backlog) {
    listen_fd_ = Socket::create_tcp();
    if (listen_fd_ < 0) {
        return false;
    }

    int opt = 1;
    ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    if (::listen(listen_fd_, backlog) < 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    if (!Socket::set_nonblocking(listen_fd_)) {
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    return true;
}

int Acceptor::accept() {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);

    int client_fd = ::accept(listen_fd_,
                             reinterpret_cast<sockaddr*>(&client_addr),
                             &len);

    if (client_fd < 0) {
        return -1;
    }

    if (!Socket::set_nonblocking(client_fd)) {
        ::close(client_fd);
        return -1;
    }

    return client_fd;
}

int Acceptor::fd() const {
    return listen_fd_;
}