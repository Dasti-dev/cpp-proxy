#include <iostream>
#include <unordered_set>

#include "core/event_loop/epoll_loop.h"
#include "core/socket/acceptor.h"
#include "core/socket/socket.h"
#include "core/fd/fd_wrapper.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

/*
 * Phase-1 FD lifecycle test
 *
 * Rules:
 * - Accept connections
 * - Immediately close client FD
 * - No buffers
 * - No connection manager
 * - No maps storing objects
 *
 * If FD leaks here → Phase 1 is broken.
 */

int main() {
    Acceptor acceptor;
    if (!acceptor.listen(8080)) {
        std::cerr << "listen failed\n";
        return 1;
    }

    EpollLoop loop;

    loop.add(
        acceptor.fd(),
        EPOLLIN,
        reinterpret_cast<void*>(static_cast<intptr_t>(acceptor.fd()))
    );

    while (true) {
        loop.wait(1000);

        for (const auto& ev : loop.events()) {
            int fd = static_cast<int>(
                reinterpret_cast<intptr_t>(ev.user_data)
            );

            if (fd == acceptor.fd()) {
                while (true) {
                    int client_fd = acceptor.accept();
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        break;
                    }

                    // Immediately close client
                    ::close(client_fd);
                }
            }
        }
    }
}
