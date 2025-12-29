#pragma once
#include <unordered_map>
#include <memory>
#include <iostream>

#include "connection.h"
#include "core/event_loop/epoll_loop.h"
#include "protocol/http/http_parser.h"

class ConnectionManager {
public:
    explicit ConnectionManager(EpollLoop& loop);

    void add_client(int fd);
    void handle_event(void* data, uint32_t events);
    void sweep_closed();

private:
    EpollLoop& loop_;
    std::unordered_map<int, std::unique_ptr<Connection>> conns_;

    void handle_client_read(Connection* c);
    void handle_backend_read(Connection* c);
    void close_connection(Connection* c);
};
