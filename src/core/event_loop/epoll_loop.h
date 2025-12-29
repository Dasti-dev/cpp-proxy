#pragma once

#include <vector>
#include <sys/epoll.h>

class EpollLoop {
public:
    EpollLoop();
    ~EpollLoop();

    void add(int fd, uint32_t events, void* data);
    void modify(int fd, uint32_t events, void* data);
    void remove(int fd);

    int wait(int timeout_ms);

    const epoll_event& event_at(int i) const;
    int ready_count() const;

private:
    int epoll_fd_;
    std::vector<epoll_event> events_;
    int ready_;
};
