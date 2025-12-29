#include <iostream>
#include <errno.h>

#include "core/event_loop/epoll_loop.h"
#include "core/socket/acceptor.h"
#include "connection/connection_manager.h"

int main() {
    Acceptor acceptor;
    acceptor.listen(8080);

    EpollLoop loop;
    ConnectionManager manager(loop);

    loop.add(acceptor.fd(), EPOLLIN, nullptr);
    std::cout << "[proxy] listening on port 8080\n";

    while (true) {
        int n = loop.wait(1000);
        if (n <= 0)
            continue;

        for (int i = 0; i < loop.ready_count(); ++i) {
            const epoll_event& ev = loop.event_at(i);

            if (ev.data.ptr == nullptr) {
                while (true) {
                    int cfd = acceptor.accept();
                    if (cfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        break;
                    }

                    std::cout << "[proxy] new client fd=" << cfd << "\n";
                    manager.add_client(cfd);
                }
            } else {
                manager.handle_event(ev.data.ptr, ev.events);
            }
        }

        manager.sweep_closed();
    }
}
