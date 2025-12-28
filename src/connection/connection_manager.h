#pragma once

#include <unordered_map>

#include "connection.h"
#include "core/event_loop/epoll_loop.h"
#include "protocol/http/http_parser.h"

/*
 * ConnectionManager
 * -----------------
 * Single-backend reverse proxy.
 *
 * epoll user_data ALWAYS points to Connection*
 */

class ConnectionManager {
public:
    explicit ConnectionManager(EpollLoop& loop);

    void add_client(int client_fd);
    void handle_event(Connection* conn, int fd, uint32_t events);

private:
    EpollLoop& loop_;
    std::unordered_map<int, Connection> connections_;
    HttpParser parser_;

    void close_connection(Connection* conn);

    // handlers
    void handle_client_read(Connection* conn);
    void handle_backend_connect(Connection* conn);
    void handle_backend_write(Connection* conn);
    void handle_backend_read(Connection* conn);
    void handle_client_write(Connection* conn);

    int connect_backend();
};
