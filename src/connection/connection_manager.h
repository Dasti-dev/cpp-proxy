#pragma once

#include <unordered_map>

#include "connection.h"
#include "core/event_loop/epoll_loop.h"

/*
 * ConnectionManager
 * -----------------
 * Owns all live connections and enforces lifecycle.
 *
 * epoll user_data invariant:
 *   - Stores ONLY client fd (int)
 *   - Never stores pointers or references
 */

class ConnectionManager {
public:
    explicit ConnectionManager(EpollLoop& loop);

    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    // Called when acceptor accepts a new client
    void add_client(int client_fd);

    // Dispatch epoll event by fd (SAFE)
    void handle_event(int fd, uint32_t events);

private:
    EpollLoop& loop_;
    std::unordered_map<int, Connection> connections_;

    // Lifecycle
    void close_connection(int fd);

    // Echo handlers (Phase 3.5 validation)
    void handle_client_reading(Connection& conn);
    void handle_client_writing(Connection& conn);
};
