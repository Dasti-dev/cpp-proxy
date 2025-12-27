#include <iostream>
#include <unordered_map>
#include <cstring>

#include "core/event_loop/epoll_loop.h"
#include "core/socket/acceptor.h"
#include "core/socket/socket.h"
#include "core/buffer/buffer.h"
#include "core/fd/fd_wrapper.h"

#include <sys/epoll.h>
#include <errno.h>

/*
 * Echo server validation
 *
 * Goals:
 * - Accept many concurrent connections
 * - Non-blocking I/O only
 * - Partial read/write handling
 * - No FD leaks
 * - CPU stable under idle load
 */

struct Client {
    FDWrapper fd;
    Buffer read_buf;
    Buffer write_buf;

    explicit Client(int fd_)
        : fd(fd_), read_buf(4096), write_buf(4096) {}
};

int main() {
    Acceptor acceptor;
    if (!acceptor.listen(8080)) {
        std::cerr << "Failed to listen\n";
        return 1;
    }

    EpollLoop loop;

    // Register listening socket
    loop.add(
        acceptor.fd(),
        EPOLLIN,
        &acceptor
    );

    std::unordered_map<int, Client> clients;

    while (true) {
        int n = loop.wait(1000);
        if (n <= 0) {
            continue;
        }

        for (const auto& ev : loop.events()) {

            /* ------------------------------
             * New incoming connections
             * ------------------------------ */
            if (ev.user_data == &acceptor) {
                while (true) {
                    int client_fd = acceptor.accept();
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        break;
                    }

                    clients.emplace(
                        client_fd,
                        Client(client_fd)
                    );

                    loop.add(
                        client_fd,
                        EPOLLIN | EPOLLOUT | EPOLLRDHUP,
                        &clients.at(client_fd)
                    );
                }
                continue;
            }

            /* ------------------------------
             * Existing client
             * ------------------------------ */
            Client* client = static_cast<Client*>(ev.user_data);
            int fd = client->fd.get();

            // Peer closed or error
            if (ev.events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
                loop.remove(fd);
                clients.erase(fd);
                continue;
            }

            /* ------------------------------
             * Read path
             * ------------------------------ */
            if (ev.events & EPOLLIN) {
                while (true) {
                    char* wptr = client->read_buf.write_ptr();
                    size_t cap = client->read_buf.writable_bytes();

                    ssize_t nread = Socket::read(fd, wptr, cap);

                    if (nread > 0) {
                        client->read_buf.commit(nread);

                        size_t bytes = client->read_buf.readable_bytes();

                        std::memcpy(
                            client->write_buf.write_ptr(),
                            client->read_buf.read_ptr(),
                            bytes
                        );

                        client->write_buf.commit(bytes);
                        client->read_buf.clear();

                    } else if (nread == 0) {
                        // Peer closed cleanly
                        loop.remove(fd);
                        clients.erase(fd);
                        break;

                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        loop.remove(fd);
                        clients.erase(fd);
                        break;
                    }
                }
            }

            /* ------------------------------
             * Write path
             * ------------------------------ */
            if (ev.events & EPOLLOUT) {
                while (client->write_buf.readable_bytes() > 0) {
                    ssize_t nwritten = Socket::write(
                        fd,
                        client->write_buf.read_ptr(),
                        client->write_buf.readable_bytes()
                    );

                    if (nwritten > 0) {
                        client->write_buf.consume(nwritten);
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        loop.remove(fd);
                        clients.erase(fd);
                        break;
                    }
                }
            }
        }
    }

    return 0;
}
