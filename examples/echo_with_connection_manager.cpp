#include <iostream>

#include "core/event_loop/epoll_loop.h"
#include "core/socket/acceptor.h"
#include "connection/connection_manager.h"

/*
 * Entry point for proxy with ConnectionManager
 *
 * epoll user_data = Connection*
 * FD is determined from Connection state
 */

int main() {
    try {
        EpollLoop loop;

        // Create listening socket
        Acceptor acceptor;
        if (!acceptor.listen(8080)) {
            std::cerr << "Failed to listen on 8080\n";
            return 1;
        }

        ConnectionManager manager(loop);

        // Register listening socket
        loop.add(
            acceptor.fd(),
            EPOLLIN,
            &acceptor   // acceptor is special-cased
        );

        std::cout << "Proxy listening on port 8080\n";

        while (true) {
            loop.wait(1000);

            for (const auto& ev : loop.events()) {

                /* ================= ACCEPT NEW CLIENT ================= */

                if (ev.user_data == &acceptor) {
                    while (true) {
                        int client_fd = acceptor.accept();
                        if (client_fd < 0) {
                            break;
                        }
                        manager.add_client(client_fd);
                    }
                    continue;
                }

                /* ================= DISPATCH CONNECTION EVENT ================= */

                Connection* conn = static_cast<Connection*>(ev.user_data);
                if (!conn) continue;

                int fd = -1;

                // Decide which FD triggered epoll based on connection state
                switch (conn->state()) {
                    case ConnectionState::CLIENT_READING_HEADERS:
                    case ConnectionState::CLIENT_WRITING_RESPONSE:
                        fd = conn->client_fd();
                        break;

                    case ConnectionState::BACKEND_CONNECTING:
                    case ConnectionState::BACKEND_WRITING_REQUEST:
                    case ConnectionState::BACKEND_READING_RESPONSE:
                        fd = conn->backend_fd();
                        break;

                    default:
                        continue;
                }

                manager.handle_event(conn, fd, ev.events);
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
