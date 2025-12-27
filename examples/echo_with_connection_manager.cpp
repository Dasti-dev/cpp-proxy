#include <iostream>

#include "core/event_loop/epoll_loop.h"
#include "core/socket/acceptor.h"
#include "connection/connection_manager.h"

#include <sys/epoll.h>
#include <errno.h>

/*
 * Echo server using SAFE ConnectionManager wiring
 */

int main() {
    Acceptor acceptor;
    if (!acceptor.listen(8080)) {
        std::cerr << "Failed to listen\n";
        return 1;
    }

    EpollLoop loop;
    ConnectionManager manager(loop);

    // Register acceptor fd
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
                    manager.add_client(client_fd);
                }
            } else {
                manager.handle_event(fd, ev.events);
            }
        }
    }
}
