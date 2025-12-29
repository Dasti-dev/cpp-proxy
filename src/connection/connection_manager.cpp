#include "connection_manager.h"
#include "core/socket/socket.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

ConnectionManager::ConnectionManager(EpollLoop& loop)
    : loop_(loop) {}

void ConnectionManager::add_client(int fd) {
    auto conn = std::make_unique<Connection>(fd);

    loop_.add(fd, EPOLLIN | EPOLLRDHUP, &conn->client_tag);

    std::cout << "[proxy] registered client fd=" << fd << "\n";
    conns_[fd] = std::move(conn);
}

void ConnectionManager::handle_event(void* data, uint32_t events) {
    auto* tag = static_cast<Connection::EpollTag*>(data);
    Connection* c = tag->conn;

    if (!c || c->is_closing())
        return;

    int fd = tag->is_client ? c->client_fd() : c->backend_fd();

    std::cout << "[proxy] epoll event fd=" << fd
              << " events=" << events
              << " state=" << static_cast<int>(c->state_) << "\n";

    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        close_connection(c);
        return;
    }

    if (tag->is_client && (events & EPOLLIN)) {
        handle_client_read(c);
    } else if (!tag->is_client && (events & EPOLLIN)) {
        handle_backend_read(c);
    }
}

void ConnectionManager::handle_client_read(Connection* c) {
    char* wptr = c->client_read_buf.write_ptr();
    size_t cap = c->client_read_buf.writable_bytes();

    ssize_t n = Socket::read(c->client_fd(), wptr, cap);
    if (n <= 0) {
        close_connection(c);
        return;
    }

    c->client_read_buf.commit(n);
    std::cout << "[proxy] read " << n << " bytes from client\n";

    HttpParser parser;
    HttpRequestInfo req;

    HttpParseResult res = parser.parse(
        c->client_read_buf.read_ptr(),
        c->client_read_buf.readable_bytes(),
        req
    );

    if (res != HttpParseResult::COMPLETE)
        return;

    std::cout << "[proxy] HTTP request COMPLETE\n";

    int bfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (bfd < 0) {
        close_connection(c);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    connect(bfd, (sockaddr*)&addr, sizeof(addr));

    c->set_backend_fd(bfd);
    loop_.add(bfd, EPOLLIN | EPOLLRDHUP, &c->backend_tag);

    std::cout << "[proxy] backend socket created fd=" << bfd << "\n";

    Socket::write(
        bfd,
        c->client_read_buf.read_ptr(),
        c->client_read_buf.readable_bytes()
    );

    c->client_read_buf.clear();
}

void ConnectionManager::handle_backend_read(Connection* c) {
    char buf[8192];

    ssize_t n = Socket::read(c->backend_fd(), buf, sizeof(buf));
    if (n <= 0) {
        std::cout << "[proxy] backend closed\n";
        close_connection(c);
        return;
    }

    std::cout << "[proxy] read " << n << " bytes from backend\n";
    Socket::write(c->client_fd(), buf, n);
}

void ConnectionManager::close_connection(Connection* c) {
    if (c->is_closing())
        return;

    std::cout << "[proxy] closing client_fd=" << c->client_fd() << "\n";

    c->mark_closing();
    loop_.remove(c->client_fd());

    if (c->backend_fd() >= 0)
        loop_.remove(c->backend_fd());
}

void ConnectionManager::sweep_closed() {
    for (auto it = conns_.begin(); it != conns_.end(); ) {
        if (it->second->is_closing())
            it = conns_.erase(it);
        else
            ++it;
    }
}
