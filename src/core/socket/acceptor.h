#pragma once

#include <cstdint>

/*
 * Acceptor
 * --------
 * Listens on a TCP port and accepts incoming connections.
 *
 * Responsibilities:
 * - Create listening socket
 * - Bind + listen
 * - Accept new connections
 * - Set accepted sockets to non-blocking
 *
 * Non-responsibilities:
 * - epoll registration
 * - connection ownership
 * - protocol logic
 */

class Acceptor {
public:
    Acceptor();
    ~Acceptor();

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    // Bind and start listening
    bool listen(uint16_t port, int backlog = 1024);

    // Accept a new connection
    // Returns:
    //  >=0 : client fd
    //   -1 : no connection or error (check errno outside)
    int accept();

    int fd() const;

private:
    int listen_fd_;
};
